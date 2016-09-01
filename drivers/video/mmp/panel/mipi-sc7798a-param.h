/* drivers/video/mmp/panel/mipi-sc7798a-param.h
 *
 * Copyright (C) 2014 Samsung Electronics Co, Ltd.
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

#ifndef __MIPI_SC7798A_PARAM_H
#define __MIPI_SC7798A_PARAM_H

#include <video/mipi_display.h>

#define HS_MODE 0
#define LP_MODE 1
#define SC7798A_SLEEP_IN_DELAY 120
#define SC7798A_DISP_OFF_DELAY 10
#define SC7798A_SLEEP_OUT_DELAY 120
#define SC7798A_DISP_ON_DELAY 10

#define MMP_DSI_CMD_DESC(type, mode, delay, data) \
	{type, mode, delay, sizeof(data), data}

#define MMP_DSI_HS_SHORT_CMD_DESC(data, delay) \
	MMP_DSI_CMD_DESC(MIPI_DSI_DCS_SHORT_WRITE, HS_MODE, delay, data)

#define MMP_DSI_HS_LONG_CMD_DESC(data, delay) \
	MMP_DSI_CMD_DESC(MIPI_DSI_DCS_LONG_WRITE, HS_MODE, delay, data)

#define MMP_DSI_LP_SHORT_CMD_DESC(data, delay) \
	MMP_DSI_CMD_DESC(MIPI_DSI_DCS_SHORT_WRITE, LP_MODE, delay, data)

#define MMP_DSI_LP_LONG_CMD_DESC(data, delay) \
	MMP_DSI_CMD_DESC(MIPI_DSI_DCS_LONG_WRITE, LP_MODE, delay, data)

static char pkt_size_cmd[] = {0x1};

static char pkt_size_cmd_4[] = {0x4};

static char read_id[] = {0x04};
static char read_id1[] = {0xda};
static char read_id2[] = {0xdb};
static char read_id3[] = {0xdc};

static char exit_sleep[] = {0x11};
static char display_on[] = {0x29};
static char sleep_in[] = {0x10};
static char display_off[] = {0x28};

static struct mmp_dsi_cmd_desc read_id_cmds[] = {
	{MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE, HS_MODE, 0,
		sizeof(pkt_size_cmd_4), pkt_size_cmd_4},
	{MIPI_DSI_DCS_READ, HS_MODE, 0, sizeof(read_id), read_id},
};

static struct mmp_dsi_cmd_desc read_id1_cmds[] = {
	{MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE, HS_MODE, 0,
		sizeof(pkt_size_cmd), pkt_size_cmd},
	{MIPI_DSI_DCS_READ, HS_MODE, 0, sizeof(read_id1), read_id1},
};

static struct mmp_dsi_cmd_desc read_id2_cmds[] = {
	{MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE, HS_MODE, 0,
		sizeof(pkt_size_cmd), pkt_size_cmd},
	{MIPI_DSI_DCS_READ, HS_MODE, 0, sizeof(read_id2), read_id2},
};

static struct mmp_dsi_cmd_desc read_id3_cmds[] = {
	{MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE, HS_MODE, 0,
		sizeof(pkt_size_cmd), pkt_size_cmd},
	{MIPI_DSI_DCS_READ, HS_MODE, 0, sizeof(read_id3), read_id3},
};

static char sc7798a_cpt_set_extension[] = {
	0xB9,
	0xF1, 0x08, 0x00,
};

static char sc7798a_cpt_set_vddd[] = {
	0xBC,
	0x67,
};

static char sc7798a_cpt_set_mipi_characteristic[] = {
	0xBA,
	0x31, 0x00, 0x44, 0x25, 0xB1,
	0x0A, 0x00, 0x00, 0xC2, 0x34,
	0x00, 0x00, 0x04, 0x02, 0x4D,
	0xF5, 0xEE, 0x40,
};

static char sc7798a_cpt_set_display_reg[] = {
	0xB2,
	0x23,
};

static char sc7798a_cpt_set_panel_inversion[] = {
	0xB4,
	0x00,
};

static char sc7798a_cpt_set_power[] = {
	0xB1,
	0x22, 0x1B, 0x1B, 0xB7, 0x22,
	0x02, 0xA8,
};

static char sc7798a_cpt_set_esd[] = {
	0xC6,
	0x00, 0x00, 0xFF,
};

static char sc7798a_cpt_set_panel[] = {
	0xCC,
	0x0C,
};

static char sc7798a_cpt_set_panel_resolution[] = {
	0xB2,
	0x23,
};

static char sc7798a_cpt_set_vcom_voltage[] = {
	0xB6,
	0x29, 0x29,
};

static char sc7798a_cpt_set_source_eq[] = {
	0xe3,
	0x02, 0x02, 0x02, 0x02,
};

static char sc7798a_cpt_set_external_power_ic[] = {
	0xb8,
	0x07, 0x22,
};

static char sc7798a_cpt_set_vref_voltage[] = {
	0xb5,
	0x09, 0x09,
};

static char sc7798a_cpt_set_source_bias[] = {
	0xc0,
	0x73, 0x50, 0x00, 0x08, 0x70,
};

static char sc7798a_cpt_set_rgb[] = {
	0xb3,
	0x01, 0x00, 0x06, 0x06, 0x10,
	0x0A, 0x45, 0x40,
};

static char sc7798a_cpt_set_gip1[] = {
	0xE9,
	0x00, 0x00, 0x08, 0x03, 0x2F,
	0x89, 0x6A, 0x12, 0x31, 0x23,
	0x48, 0x0C, 0x89, 0x6A, 0x47,
	0x02, 0x04, 0x00, 0x00, 0x00,
	0x00, 0x20, 0x88, 0x88, 0x40,
	0x28, 0x69, 0x48, 0x88, 0x88,
	0x80, 0x88, 0x88, 0x51, 0x38,
	0x79, 0x58, 0x88, 0x88, 0x81,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
};

static char sc7798a_cpt_set_gip2[] = {
	0xea,
	0x88, 0x88, 0x37, 0x59, 0x18,
	0x18, 0x88, 0x88, 0x85, 0x88,
	0x88, 0x26, 0x49, 0x08, 0x08,
	0x88, 0x88, 0x84, 0x30, 0x00,
	0x00, 0xFF, 0x00, 0x50, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
};

static char sc7798a_cpt_set_gamma_curve[] = {
	0xe0,
	0x00, 0x00, 0x00, 0x04, 0x04,
	0x0A, 0x18, 0x2B, 0x05, 0x0C,
	0x11, 0x16, 0x18, 0x16, 0x16,
	0x15, 0x19, 0x00, 0x00, 0x00,
	0x04, 0x04, 0x0A, 0x18, 0x2C,
	0x05, 0x0C, 0x12, 0x16, 0x18,
	0x16, 0x17, 0x16, 0x19,
};

static char sc7798a_cpt_set_dgc_lut[] = {
	0xc1,
	0x01, 0x00, 0x08, 0x10, 0x18,
	0x20, 0x26, 0x30, 0x36, 0x3F,
	0x48, 0x50, 0x58, 0x60, 0x69,
	0x71, 0x79, 0x82, 0x89, 0x91,
	0x99, 0xA1, 0xA9, 0xB1, 0xB9,
	0xC1, 0xC9, 0xD1, 0xD9, 0xE0,
	0xE8, 0xF0, 0xF2, 0xF4, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08,
	0x10, 0x18, 0x20, 0x27, 0x30,
	0x37, 0x40, 0x48, 0x50, 0x58,
	0x60, 0x68, 0x70, 0x78, 0x80,
	0x88, 0x90, 0x98, 0xA0, 0xA8,
	0xB0, 0xB8, 0xC0, 0xC8, 0xD0,
	0xD8, 0xE0, 0xE8, 0xF0, 0xF8,
	0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x08, 0x12, 0x1A, 0x23,
	0x29, 0x31, 0x38, 0x41, 0x48,
	0x4F, 0x57, 0x5E, 0x66, 0x6C,
	0x74, 0x7B, 0x84, 0x8B, 0x94,
	0x9B, 0xA4, 0xAb, 0xB4, 0xBB,
	0xC4, 0xCD, 0xD9, 0xE0, 0xE8,
	0xF0, 0xF8, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

static struct mmp_dsi_cmd_desc sc7798a_cpt_init_cmds[] = {
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_extension, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_vddd, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_mipi_characteristic, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_display_reg, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_panel_inversion, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_power, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_esd, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_panel, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_source_eq, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_external_power_ic, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_vref_voltage, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_source_bias, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_rgb, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_extension, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_gip1, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_gip2, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_cpt_set_gamma_curve, 0),
};

/* ############### BOE ############### */
static char sc7798a_boe_B9h[] = {
	0xB9,
	0xF1, 0x08, 0x00,
};

static char sc7798a_boe_B1h[] = {
	0xB1,
	0x32, 0x0C, 0x0C, 0x22, 0x33,
	0x01, 0xB7,
};

static char sc7798a_boe_E3h[] = {
	0xE3,
	0x05, 0x05, 0x05, 0x05,
};

static char sc7798a_boe_B8h[] = {
	0xB8,
	0x06, 0x22,
};

static char sc7798a_boe_BCh[] = {
	0xBC,
	0x67,
};

static char sc7798a_boe_C0h[] = {
	0xC0,
	0x73, 0x50, 0x00, 0x08, 0x70,
};

static char sc7798a_boe_BAh[] = {
	0xBA,
	0x31, 0x00, 0x44, 0x25, 0x91,
	0x0A, 0x00, 0x00, 0xC2, 0x34,
	0x00, 0x00, 0x04, 0x02, 0x1D,
	0xB9, 0xEE, 0x40,
};

static char sc7798a_boe_C6h[] = {
	0xC6,
	0x00, 0x00, 0xFF,
};

static char sc7798a_boe_CCh[] = {
	0xCC,
	0x0C,
};

static char sc7798a_boe_B2h[] = {
	0xB2,
	0x03,
};

static char sc7798a_boe_B4h[] = {
	0xB4,
	0x02,
};

static char sc7798a_boe_B3h[] = {
	0xB3,
	0x00, 0x00, 0x00, 0x00, 0x10,
	0x0A, 0x25, 0x20,
};

static char sc7798a_boe_E9h[] = {
	0xE9,
	0x00, 0x00, 0x05, 0x00, 0x00,
	0x47, 0x43, 0x12, 0x30, 0x00,
	0x38, 0x08, 0x47, 0x43, 0x47,
	0x00, 0x60, 0x00, 0x00, 0x00,
	0x00, 0x1C, 0x88, 0x88, 0x86,
	0x64, 0x42, 0x20, 0x00, 0x29,
	0x88, 0x88, 0x88, 0x87, 0x75,
	0x53, 0x31, 0x11, 0x39, 0x88,
	0x00, 0x00, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
};

static char sc7798a_boe_EAh[] = {
	0xEA,
	0x88, 0x88, 0x91, 0x13, 0x35,
	0x57, 0x73, 0x18, 0x88, 0x88,
	0x88, 0x90, 0x02, 0x24, 0x46,
	0x62, 0x08, 0x88, 0x03, 0x01,
	0x00, 0xFF, 0x00, 0x90, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
};

static char sc7798a_boe_B0h[] = {
	0xB0,
	0x01,
};

static struct mmp_dsi_cmd_desc sc7798a_boe_init_cmds[] = {
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_B9h, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_B1h, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_E3h, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_B8h, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_BCh, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_C0h, 10),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_BAh, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_C6h, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_CCh, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_B2h, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_B4h, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_B3h, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_E9h, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_EAh, 0),
	MMP_DSI_HS_LONG_CMD_DESC(sc7798a_boe_B0h, 0),
};

static struct mmp_dsi_cmd_desc sc7798a_video_power_on_cmds[] = {
	MMP_DSI_HS_SHORT_CMD_DESC(exit_sleep, SC7798A_SLEEP_OUT_DELAY),
	MMP_DSI_HS_SHORT_CMD_DESC(display_on, SC7798A_DISP_ON_DELAY),
};

static struct mmp_dsi_cmd_desc sc7798a_video_power_off_cmds[] = {
	MMP_DSI_HS_SHORT_CMD_DESC(display_off, SC7798A_DISP_OFF_DELAY),
	MMP_DSI_HS_SHORT_CMD_DESC(sleep_in, SC7798A_SLEEP_IN_DELAY),
};

#endif /* __MIPI_SC7798A_PARAM_H */