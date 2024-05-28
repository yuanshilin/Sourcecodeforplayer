/*
 *Copyright (c) 2023 UWA(UHD World Association)
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

#ifndef AVFORMAT_HDRVIVID_ISOM_H
#define AVFORMAT_HDRVIVID_ISOM_H

#include "avformat.h"
#include "libavutil/hdr_dynamic_vivid_metadata.h"

#define ISOM_HDRVIVID_CUVV_SIZE 22

int ff_isom_parse_cuvv(AVFormatContext *s, AVStream *st, const uint16_t *buf_ptr, uint64_t size);
void ff_isom_put_cuvv(AVFormatContext *s, uint8_t out[ISOM_HDRVIVID_CUVV_SIZE],
                           AVHDRVIVIDDecoderCUVVRecord *vivid);

#endif /* AVFORMAT_HDRVIVID_ISOM_H */
