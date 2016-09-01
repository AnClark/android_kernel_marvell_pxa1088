/* drivers/video/mmp/panel/mipi-nt35510.c
 *
 * Copyright (C) 2013 Samsung Electronics Co, Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/lcd.h>
#include <linux/backlight.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <video/mmp_disp.h>
#include <video/mipi_display.h>
#include "mipi-nt35510-param.h"
#include <linux/gpio.h>
#include <linux/platform_data/panel-nt35510.h>

/* PMIC Regulator based supply to LCD */
#define REGULATOR_SUPPLY	1
/* gpio controlled LDO based supply to LCD */
#define LDO_SUPPLY			0

struct nt35510 {

	struct device *dev;
	struct class *lcd_class;
	struct nt35510_panel_data  *pdata;
	struct backlight_device *bd;
	u32 (*set_panel_id)(struct mmp_panel *panel);
	/* Further fields can be added here */
	int lcd_rst_n;
	int lcd_bl_n;
	int lcd_en_gpio1;
	int lcd_en_gpio2;
	u32 lcd_iovdd_type;
	u32 lcd_avdd_type;


};

static u32 boot_panel_id;

static int __init get_boot_panel_id(char *str)
{
	char *endp;

	boot_panel_id = memparse(str, &endp);
	pr_info("%s: boot_panel_id = 0x%8x\n", __func__, boot_panel_id);

	return 1;
}
early_param("lcd_id", get_boot_panel_id);

static int tps61158_send_bit(int gpio_bl, bool bit)
{
	if (bit) { /* Send bit 1 */
		gpio_direction_output(gpio_bl, 0);
		udelay(TPS61158_TIME_DATA_DELAY_SHORT);
		gpio_direction_output(gpio_bl, 1);
		udelay(TPS61158_TIME_DATA_DELAY_LONG);
	} else { /* Send bit 0 */
		gpio_direction_output(gpio_bl, 0);
		udelay(TPS61158_TIME_DATA_DELAY_LONG);
		gpio_direction_output(gpio_bl, 1);
		udelay(TPS61158_TIME_DATA_DELAY_SHORT);
	}

	return 0;
}

static int tps61158_send_tstart_signal(int gpio_bl)
{
	gpio_direction_output(gpio_bl, 0);
	udelay(TPS61158_TIME_T_EOS);
	gpio_direction_output(gpio_bl, 1);
	udelay(TPS61158_TIME_ES_DET_DELAY);
	gpio_direction_output(gpio_bl, 0);
	udelay(TPS61158_TIME_ES_DET_TIME);
	gpio_direction_output(gpio_bl, 1);
	mdelay(3);
	udelay(500); /* ES window time */
	return 0;
}

static int tps61158_send_brightness_signal(int gpio_bl,
		unsigned int brightness_index)
{
	int i;

	tps61158_send_tstart_signal(gpio_bl); /* FLAG */

	for (i = 0; i < TPS61158_ADDRESS_BITS; i++) /* Send Address */
		tps61158_send_bit(gpio_bl,
				tps61158_table[brightness_index].bl_send_address
				& (0x1 << i));

	gpio_direction_output(gpio_bl, 0); /* 2nd FLAG */
	udelay(TPS61158_TIME_T_EOS);
	gpio_direction_output(gpio_bl, 1);
	udelay(TPS61158_TIME_T_START);

	for (i = 0; i < TPS61158_DATA_BITS; i++) /* Send DATA */
		tps61158_send_bit(gpio_bl,
				tps61158_table[brightness_index].bl_send_data
				& (0x1 << i));

	gpio_direction_output(gpio_bl, 0); /* 3rd FLAG */
	udelay(TPS61158_TIME_T_EOS);
	gpio_direction_output(gpio_bl, 1); /* BL brightness enable */
	return 0;
}

static int tps61158_brightness_update(int gpio_backlight, int intensity)
{
	int bl_level = -1;
	int i, bl_value;

	/*
	 *
	 * FIXME
	 * the initial value of bl_level_last is the
	 * uboot backlight level, it should be aligned.
	 */
	static int bl_level_last = -1;

	/* Brightness is controlled by a series of pulses
	 * generated by gpio. It has 32 leves and level 1
	 * is the brightest. Pull low for 3ms makes
	 * backlight shutdown
	 */
	if (intensity < 0 || intensity > MAX_BRIGHTNESS) {
		pr_info("%s, %d is exceeded max brightness\n",
				__func__, MAX_BRIGHTNESS);
		return -EINVAL;
	}

	bl_value = intensity * 255 / MAX_BRIGHTNESS;
	for (i = 0; i < MAX_BRIGHTNESS_IN_BLU; i++) {
		if (bl_value >= tps61158_table[i].bl_value)
			bl_level = tps61158_table[i].bl_level;
		else
			break;

		pr_info("intensity : %d, bl_level : %d\n",
		intensity, bl_level);
	}

	if (bl_level == bl_level_last)
		goto set_bl_return;

	if (!bl_level) {
		/* shutdown backlight */
		gpio_set_value(gpio_backlight, 0);
		mdelay(3);
		goto set_bl_return;
	}

	if (bl_level_last < 0) {
		/* if BLIC is unknown state, reset it */
		gpio_set_value(gpio_backlight, 0);
		mdelay(3);
	}

	tps61158_send_brightness_signal(gpio_backlight, bl_level);

set_bl_return:
	bl_level_last = bl_level;
	return 0;
}

static DEFINE_SPINLOCK(bl_lock);

static void nt35510_set_bl(int gpio_bl, int intensity)
{
	spin_lock(&bl_lock);
	tps61158_brightness_update(gpio_bl, intensity);
	spin_unlock(&bl_lock);
	pr_debug("%s, intensity:%d\n", __func__, intensity);
	return;
}

static u32 nt35510_set_panel_id(struct mmp_panel *panel)
{
	u32 read_id = 0;

	struct mmp_path *path = mmp_get_path(panel->plat_path_name);
	struct mmp_dsi_buf *dbuf;

	dbuf = kmalloc(sizeof(struct mmp_dsi_buf), GFP_KERNEL);
	if (dbuf) {
		mmp_phy_dsi_rx_cmd_array(path->phy, dbuf, read_id_cmds,
				ARRAY_SIZE(read_id_cmds));
		mmp_phy_dsi_rx_cmd_array(path->phy, dbuf, read_id1_cmds,
				ARRAY_SIZE(read_id1_cmds));
		read_id |= dbuf->data[0] << 16;
		mmp_phy_dsi_rx_cmd_array(path->phy, dbuf, read_id2_cmds,
				ARRAY_SIZE(read_id2_cmds));
		read_id |= dbuf->data[0] << 8;
		mmp_phy_dsi_rx_cmd_array(path->phy, dbuf, read_id3_cmds,
				ARRAY_SIZE(read_id3_cmds));
		read_id |= dbuf->data[0];
		kfree(dbuf);
		pr_info("Panel id is 0x%x\n", read_id);
	} else
		pr_err("%s: can't alloc dsi rx buffer\n", __func__);

	return read_id;
}


static int nt35510_panel_enable(struct mmp_path *path)
{
	mmp_phy_dsi_tx_cmd_array(path->phy, nt35510_video_init_cmds,
			ARRAY_SIZE(nt35510_video_init_cmds));

	mmp_phy_dsi_tx_cmd_array(path->phy, nt35510_video_power_on_cmds,
			ARRAY_SIZE(nt35510_video_power_on_cmds));

	return 0;
}

static int nt35510_panel_disable(struct mmp_path *path)
{
	mmp_phy_dsi_tx_cmd_array(path->phy, nt35510_video_power_off_cmds,
			ARRAY_SIZE(nt35510_video_power_off_cmds));

	return 0;
}

#ifdef CONFIG_OF
static void nt35510_panel_power(struct mmp_panel *panel, int skip_on, int on)
{
	static struct regulator *lcd_iovdd;
	static struct regulator *lcd_avdd;
	int err;
	struct nt35510 *plat = panel->plat_data;
	int lcd_rst_n = plat->lcd_rst_n;


	if (plat->lcd_iovdd_type == REGULATOR_SUPPLY) {
		if (!lcd_iovdd) {
			lcd_iovdd = regulator_get(panel->dev, "iovdd");
			if (IS_ERR_OR_NULL(lcd_iovdd)) {
				pr_err("%s: regulatori(iovdd- 1.8v) get error!\n",
						__func__);
				goto err_lcd_iovdd_type;
			}
		}
	}

	if ((plat->lcd_avdd_type == REGULATOR_SUPPLY)) {
		/* Regulator V_LCD_3.0V */
		if (!lcd_avdd) {
			lcd_avdd = regulator_get(panel->dev, "avdd");
			if (IS_ERR_OR_NULL(lcd_avdd)) {
				pr_err("%s regulator(avdd - 3.0v) get error!\n",
						__func__);
				goto err_lcd_avdd_type;
			}
		}
	}

	if (on) {
		if (plat->lcd_avdd_type == REGULATOR_SUPPLY) {
			err = regulator_enable(lcd_avdd);
			if (unlikely(err < 0)) {
				pr_err("%s: lcd_avdd regulator enable failed %d\n",
						__func__, err);
				goto err_lcd_avdd_enable_failed;
			}
			mdelay(5);
		}

		if (plat->lcd_iovdd_type == REGULATOR_SUPPLY) {
			err = regulator_enable(lcd_iovdd);
			if (unlikely(err < 0)) {
				pr_err("%s: lcd_iovdd regulator enable failed %d\n",
						__func__, err);
				goto err_lcd_iovdd_enable_failed;
			}
			mdelay(15);
		}

		if (plat->lcd_iovdd_type == LDO_SUPPLY) {
			if (plat->lcd_en_gpio1)
				gpio_direction_output(plat->lcd_en_gpio1, 1);
		}

		if (plat->lcd_avdd_type == LDO_SUPPLY) {
			if ((plat->lcd_en_gpio2) &&
				(plat->lcd_en_gpio1 != plat->lcd_en_gpio2))
				gpio_direction_output(plat->lcd_en_gpio2, 1);
		}

		if (!skip_on) {
			gpio_direction_output(lcd_rst_n, 1);
			udelay(20);
			gpio_direction_output(lcd_rst_n, 0);
			udelay(2000);
			gpio_direction_output(lcd_rst_n, 1);
			mdelay(150);
		}

	} else {
		/* set panel reset */
		gpio_direction_output(lcd_rst_n, 0);

		if (plat->lcd_iovdd_type == REGULATOR_SUPPLY) {
			/* disable V_LCD_1.8V */
			regulator_disable(lcd_iovdd);
			mdelay(10);
		}

		if (plat->lcd_avdd_type == REGULATOR_SUPPLY)
			/* disable V_LCD_3.0V */
			regulator_disable(lcd_avdd);

		 if (plat->lcd_avdd_type == LDO_SUPPLY) {
			if ((plat->lcd_en_gpio2) &&
				(plat->lcd_en_gpio1 != plat->lcd_en_gpio2))
				gpio_direction_output(plat->lcd_en_gpio2, 0);
		 }

		 if (plat->lcd_iovdd_type == LDO_SUPPLY) {
			if (plat->lcd_en_gpio1)
				gpio_direction_output(plat->lcd_en_gpio1, 0);
		}
	}
	return;
err_lcd_iovdd_enable_failed:
	if (plat->lcd_avdd_type == REGULATOR_SUPPLY)
		regulator_disable(lcd_avdd);
err_lcd_avdd_enable_failed:
	if (plat->lcd_avdd_type == REGULATOR_SUPPLY)
		regulator_put(lcd_avdd);
err_lcd_avdd_type:
	if (plat->lcd_avdd_type == REGULATOR_SUPPLY)
		lcd_avdd = NULL;
	if (plat->lcd_iovdd_type == REGULATOR_SUPPLY)
		regulator_put(lcd_iovdd);

err_lcd_iovdd_type:
	if (plat->lcd_iovdd_type == REGULATOR_SUPPLY)
		lcd_iovdd = NULL;

	return;
}
#else
static void nt35510_panel_power(struct mmp_panel *panel, int on) {}
#endif

#ifdef CONFIG_OF
static void of_nt35510_set_panel_id(struct mmp_panel *panel)
{
	struct nt35510 *plat = panel->plat_data;
	struct mmp_path *path = mmp_get_path(panel->plat_path_name);

	if (boot_panel_id)
		path->panel->id = boot_panel_id;
	else {
		if (!path->panel->id && plat->set_panel_id)
			path->panel->id = plat->set_panel_id(panel);
	}
}
#else
static void of_nt35510_set_panel_id(struct mmp_panel *panel) {}
#endif

static void nt35510_onoff(struct mmp_panel *panel, int status)
{
	struct nt35510 *plat = panel->plat_data;
	struct mmp_path *path = mmp_get_path(panel->plat_path_name);

	pr_info("called %s with status %d\n", __func__, status);

	if (status) {
		/* power on */
		if ((plat->pdata) && (plat->pdata->mi->plat_set_onoff))
			plat->pdata->mi->plat_set_onoff(1);
		else
			nt35510_panel_power(panel, 0, 1);

		if ((plat->pdata) && (plat->set_panel_id)) {
			if (boot_panel_id)
				plat->pdata->panel_id = boot_panel_id;
			else
				plat->pdata->panel_id = plat->set_panel_id(panel);
		} else
			of_nt35510_set_panel_id(panel);

		nt35510_panel_enable(path);

	} else {
		/* power off */
		nt35510_panel_disable(path);
		if ((plat->pdata) && (plat->pdata->mi->plat_set_onoff))
			plat->pdata->mi->plat_set_onoff(0);
		else
			nt35510_panel_power(panel, 0, 0);
	}
}

static void nt35510_reduced_onoff(struct mmp_panel *panel, int status)
{
	if (status) {
		/* power on */
		nt35510_panel_power(panel, 1, 1);
	} else {
		/* power off */
		nt35510_panel_power(panel, 1, 0);
	}
}

static struct mmp_mode mmp_modes_nt35510[] = {
	[0] = {
		.pixclock_freq = 26764320,
		.refresh = 60,
#if defined(CONFIG_MMP_VIRTUAL_RESOLUTION)
		.xres = CONFIG_MMP_VIRTUAL_RESOLUTION_X,
		.yres = CONFIG_MMP_VIRTUAL_RESOLUTION_Y,
#else
		.xres = 480,
		.yres = 800,
#endif
		.real_xres = 480,
		.real_yres = 800,
		.hsync_len = 4,
		.left_margin = 32,
		.right_margin = 32,
		.vsync_len = 2,
		.upper_margin = 8,
		.lower_margin = 4,
		.invert_pixclock = 0,
		.pix_fmt_out = PIXFMT_RGB888PACK,
		.hsync_invert = 0,
		.vsync_invert = 0,
	},
};

static int nt35510_get_modelist(struct mmp_panel *panel,
		struct mmp_mode **modelist)
{
	*modelist = mmp_modes_nt35510;
	return 1;
}

static struct mmp_panel panel_nt35510 = {
	.name = "nt35510",
	.panel_type = PANELTYPE_DSI_VIDEO,
	.get_modelist = nt35510_get_modelist,
	.set_onoff = nt35510_onoff,
	.reduced_onoff = nt35510_reduced_onoff,
};

static int nt35510_get_brightness(struct backlight_device *bd)
{
	    return bd->props.brightness;
}


static int nt35510_set_brightness(struct backlight_device *bd)
{
	struct nt35510 *lcd = bl_get_data(bd);
	nt35510_set_bl(lcd->lcd_bl_n, bd->props.brightness);
	return 0;
}

static const struct backlight_ops nt35510_backlight_ops = {

	.get_brightness = nt35510_get_brightness,
	.update_status = nt35510_set_brightness,
};

static ssize_t lcd_panel_name_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "NT35510");
}

static DEVICE_ATTR(lcd_panel_name, S_IRUGO | S_IRUSR | S_IRGRP | S_IXOTH,
			lcd_panel_name_show, NULL);

static ssize_t lcd_type_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct mmp_path *path = mmp_get_path(panel_nt35510.plat_path_name);
	u32 panel_id;

#ifdef CONFIG_OF
	panel_id = path->panel->id;
#else
	struct nt35510 *lcd = dev_get_drvdata(dev);
	panel_id = lcd->pdata->panel_id;
#endif
	return sprintf(buf, "NT_%x%x%x",
			(panel_id >> 16) & 0xFF,
			(panel_id >> 8) & 0xFF,
			panel_id  & 0xFF);
}

static DEVICE_ATTR(lcd_type, S_IRUGO | S_IRUSR | S_IRGRP | S_IXOTH,
			lcd_type_show, NULL);

static int nt35510_probe(struct platform_device *pdev)
{

	struct nt35510 *lcd;
	int ret;
	const char *path_name;
	struct device_node *np = pdev->dev.of_node;

	struct backlight_properties props = {
		.brightness = DEFAULT_BRIGHTNESS,
		.max_brightness = 100,
		.type = BACKLIGHT_RAW,
	};

	lcd = kzalloc(sizeof(*lcd), GFP_KERNEL);
	if (unlikely(!lcd)) {
		ret =  -ENOMEM;
		return ret;
	}

	if (IS_ENABLED(CONFIG_OF)) {
		ret = of_property_read_string(np,
				"marvell,path-name", &path_name);
		if (ret < 0)
			return ret;

		panel_nt35510.plat_path_name = path_name;

	} else {
		if (unlikely(pdev->dev.platform_data == NULL)) {
			dev_err(&pdev->dev, "no platform data!\n");
			ret = -EINVAL;
			goto err_no_platform_data;
		}

		lcd->pdata = pdev->dev.platform_data;
		panel_nt35510.plat_path_name = lcd->pdata->mi->plat_path_name;
	}

	panel_nt35510.plat_data = lcd;
	lcd->set_panel_id = &nt35510_set_panel_id;
	panel_nt35510.dev = &pdev->dev;

	lcd->lcd_rst_n = of_get_named_gpio(np, "rst-gpio", 0);
	if (lcd->lcd_rst_n < 0) {
		pr_err("%s: of_get_named_gpio failed: %d\n", __func__,
		       lcd->lcd_rst_n);
		goto err_no_platform_data;
	}

	lcd->lcd_bl_n = of_get_named_gpio(np, "bl-gpio", 0);
	if (lcd->lcd_bl_n < 0) {
		pr_err("%s: of_get_named_gpio failed: %d\n", __func__,
		       lcd->lcd_bl_n);
		goto err_no_platform_data;
	}

	pr_info("%s [%d] lcd_rst_n:%d lcd_bl_n:%d\n",
			__func__, __LINE__,
			lcd->lcd_rst_n, lcd->lcd_bl_n);

	ret = of_property_read_u32(np, "iovdd-supply-type",
			&lcd->lcd_iovdd_type);
	if (ret < 0) {
		pr_err("%s: failed to read property iovdd-supply-type\n",
				__func__);
		goto err_no_platform_data;
	}

	ret = of_property_read_u32(np, "avdd-supply-type", &lcd->lcd_avdd_type);
	if (ret < 0) {
		pr_err("%s: failed to read property avdd-supply-type\n",
				__func__);
		goto err_no_platform_data;
	}

	ret = gpio_request(lcd->lcd_rst_n, "lcd reset gpio");
	if (ret < 0) {
		pr_err("%s: gpio_request %d failed: %d\n",
				__func__, lcd->lcd_rst_n, ret);
		goto err_no_platform_data;
	}

	ret = gpio_request(lcd->lcd_bl_n, "lcd backlight");
	if (ret < 0) {
		pr_err("%s: gpio_request %d failed: %d\n",
				__func__, lcd->lcd_bl_n, ret);
		goto err_bl_gpio_failed;
	}

	if (!lcd->lcd_iovdd_type) {
		lcd->lcd_en_gpio1 = of_get_named_gpio(np, "ldo_en_gpio1", 0);
		if (lcd->lcd_en_gpio1 < 0) {
			pr_err("%s: of_get_named_gpio failed: %d\n", __func__,
					lcd->lcd_en_gpio1);
			goto err_ldo1_gpio_failed;
		}

		ret = gpio_request(lcd->lcd_en_gpio1, "lcd_ldo_en1");
		if (ret < 0) {
			pr_err("%s: gpio_request %d failed: %d\n",
				__func__, lcd->lcd_en_gpio1, ret);
			goto err_ldo1_gpio_failed;
		}
	}

	if (!lcd->lcd_avdd_type) {
		lcd->lcd_en_gpio2 = of_get_named_gpio(np, "ldo_en_gpio2", 0);
		if (lcd->lcd_en_gpio2 < 0) {
			pr_err("%s: of_get_named_gpio failed: %d\n", __func__,
					lcd->lcd_en_gpio2);
			goto err_ldo2_gpio_failed;
		}

		if (lcd->lcd_en_gpio1 != lcd->lcd_en_gpio2) {
			ret = gpio_request(lcd->lcd_en_gpio2, "lcd_ldo_en2");
			if (ret < 0) {
				pr_err("%s: gpio_request %d failed: %d\n",
					__func__, lcd->lcd_en_gpio2, ret);
				goto err_ldo2_gpio_failed;
			}
		}

	}

	lcd->lcd_class = class_create(THIS_MODULE, "lcd");
	if (IS_ERR(lcd->lcd_class)) {
		ret = PTR_ERR(lcd->lcd_class);
		pr_err("Failed to create lcd_class!");
		goto err_class_create;
	}

	lcd->dev = device_create(lcd->lcd_class, NULL, 0, "%s", "panel");
	if (IS_ERR(lcd->dev)) {
		ret = PTR_ERR(lcd->dev);
		pr_err("Failed to create device(panel)!\n");
		goto err_device_create;
	}

	dev_set_drvdata(lcd->dev, lcd);

	/*later backlight device name needs to be changed as panel */
	lcd->bd = backlight_device_register("lcd-bl", &pdev->dev, lcd,
			&nt35510_backlight_ops, &props);

	if (IS_ERR(lcd->bd)) {
		ret = PTR_ERR(lcd->bd);
		goto err_backlight_device_register;
	}

	ret = device_create_file(lcd->dev, &dev_attr_lcd_panel_name);
	if (unlikely(ret < 0)) {
		pr_err("Failed to create device file(%s)!\n",
				dev_attr_lcd_panel_name.attr.name);
		 goto err_lcd_device;
	}

	ret = device_create_file(lcd->dev, &dev_attr_lcd_type);
	if (unlikely(ret < 0)) {
		pr_err("Failed to create device file(%s)!\n",
			dev_attr_lcd_type.attr.name);
		goto err_lcd_name;
	}

	mmp_register_panel(&panel_nt35510);
	return 0;

err_lcd_name:
	 device_remove_file(lcd->dev, &dev_attr_lcd_panel_name);
err_lcd_device:
	 backlight_device_unregister(lcd->bd);
err_backlight_device_register:
	device_destroy(lcd->lcd_class, 0);
err_device_create:
	class_destroy(lcd->lcd_class);
err_ldo2_gpio_failed:
	if (!lcd->lcd_iovdd_type)
		gpio_free(lcd->lcd_en_gpio1);
err_ldo1_gpio_failed:
	gpio_free(lcd->lcd_bl_n);
err_bl_gpio_failed:
	gpio_free(lcd->lcd_rst_n);
err_class_create:
err_no_platform_data:
	kfree(lcd);

	return ret;

}

static int nt35510_remove(struct platform_device *dev)
{
	struct nt35510 *lcd = panel_nt35510.plat_data;

	if (lcd->lcd_avdd_type == LDO_SUPPLY) {
		if (lcd->lcd_en_gpio1 != lcd->lcd_en_gpio2)
			gpio_free(lcd->lcd_en_gpio2);
	}

	if (lcd->lcd_iovdd_type == LDO_SUPPLY)
		gpio_free(lcd->lcd_en_gpio1);

	gpio_free(lcd->lcd_bl_n);
	gpio_free(lcd->lcd_rst_n);

	device_remove_file(lcd->dev, &dev_attr_lcd_panel_name);
	device_remove_file(lcd->dev, &dev_attr_lcd_type);
	backlight_device_unregister(lcd->bd);
	device_destroy(lcd->lcd_class, 0);
	class_destroy(lcd->lcd_class);

	mmp_unregister_panel(&panel_nt35510);
	kfree(lcd);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id mmp_nt35510_dt_match[] = {
	{ .compatible = "marvell,mmp-nt35510" },
	{},
};
#endif

static struct platform_driver nt35510_driver = {
	    .driver     = {
			.name   = "mmp-nt35510",
			.owner  = THIS_MODULE,
#ifdef CONFIG_OF
			.of_match_table = of_match_ptr(mmp_nt35510_dt_match),
#endif
		},
		.probe      = nt35510_probe,
		.remove     = nt35510_remove,
};

static int nt35510_module_init(void)
{
	return platform_driver_register(&nt35510_driver);
}
static void nt35510_module_exit(void)
{
	    platform_driver_unregister(&nt35510_driver);
}
module_init(nt35510_module_init);
module_exit(nt35510_module_exit);

MODULE_DESCRIPTION("Panel driver for MIPI panel NT35510");
MODULE_LICENSE("GPL");