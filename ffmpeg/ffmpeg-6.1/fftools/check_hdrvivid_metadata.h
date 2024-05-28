/*
 * Copyright (c) -2023 UWA(UHD World Association)
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * hdr vivid metadata file integrity verifier
 */
#ifndef __CHECK_HDRVIVID_METADATA_H__
#define __CHECK_HDRVIVID_METADATA_H__

#define HDRVIVID_SIDE_DATA_LEN 60 //assume this is the maxmium bbinary metadata byte size
#define MAX_BINARY_METADATA_LEN 60

static const int32_t maxrgb_den = 4095;
static const int32_t color_saturation_gain_den = 128;
static const int32_t maximum_luminance_den = 4095;
static const int32_t base_param_m_p_den = 16383;
static const int32_t base_param_m_m_den = 63/*10*/; //in dynamic_hdr_vivid.c it is 10. but we should use 63 instead, because it is 6 bit length data
static const int32_t base_param_m_a_den = 1023;
static const int32_t base_param_m_b_den = 1023;
static const int32_t base_param_m_n_den = 63/*10*/;
static const int32_t base_param_Delta_den = 127;

/*below defines code are from hdrvivid reference code Defines.H*/
#define system_start_code_BIT        8
#define minimum_maxrgb_BIT           12
#define average_maxrgb_BIT           12
#define variance_maxrgb_BIT          12
#define maximum_maxrgb_BIT           12
#define tone_mapping_mode_BIT        1
#define tone_mapping_param_num_BIT   1
#define targeted_system_display_BIT  12
#define Base_flag_BIT                1
#define Base_param_m_p_BIT           14
#define Base_param_m_m_BIT           6
#define Base_param_m_a_BIT           10
#define Base_param_m_b_BIT           10
#define Base_param_m_n_BIT           6
#define Base_param_K1_BIT            2
#define Base_param_K2_BIT            2
#define Base_param_K3_BIT            4
#define Base_param_Delta_mode_BIT    3
#define Base_param_Delta_BIT         7
#define P3Spline_flag_BIT            1
#define P3Spline_num_BIT             1
#define P3Spline_TH_mode_BIT         2
#define P3Spline_TH_MB_BIT           8
#define P3Spline_TH_OFFSET_BIT       2
#define P3Spline_TH1_BIT             12
#define P3Spline_TH2_BIT             10
#define P3Spline_TH3_BIT             10
#define P3Spline_Strength_BIT        8
#define color_saturation_BIT         1
#define color_saturation_num_BIT     3
#define color_saturation_gain_BIT    8


char *vmeta_read_one_line(FILE *fp);
int check_cmd(int c,char **s);
bool is_empty_line(char *s);
bool is_valid_line(char * line,int linenum);
int check_vivid_metadata_file(char * s,int linetocheck,bool show_metadata_details);
int get_vivid_metadata_from_one_line_string(char *s, uint8_t *data, int *size);
int check_value(char *token,int min,int max,int linenum,int count,const char *name ,bool show_metadata_details);
int check_vivid_metadata(char *s,int linenum,bool show_metadata_details);
#endif
