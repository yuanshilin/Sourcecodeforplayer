/*
 * Copyright (c) 2023 UWA(UHD World Association)
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

#ifndef AVCODEC_VMETA_H
#define AVCODEC_VMETA_H

#include <stddef.h>
#include <stdint.h>

#include "libavutil/buffer.h"
#include "libavutil/frame.h"

/**
 * Check AVFrame for HDR Vivid  side data and allocate and fill SEI message withi Vivid info
 *
 * @param frame      Raw frame to get Vivid side data from
 * @param prefix_len Number of bytes to allocate before SEI message
 * @param data       Pointer to a variable to store allocated memory
 *                   Upon return the variable will hold NULL on error or if frame has no A53 info.
 *                   Otherwise it will point to prefix_len uninitialized bytes followed by
 *                   *sei_size SEI message
 * @param sei_size   Pointer to a variable to store generated SEI message length
 * @return           Zero on success, negative error code on failure
 */
int ff_alloc_vmeta_sei(const AVFrame *frame, size_t prefix_len,
                     void **data, size_t *sei_size);

/**
 * Parse a data array for HDR Vivid metadata and store them in an AVBufferRef.
 *
 * @param pbuf Pointer to an AVBufferRef to append the metadata. *pbuf may be NULL, in
 *             which case a new buffer will be allocated and put in it.
 * @param data The data array containing the raw A53 data.
 * @param size Size of the data array in bytes.
 *
 * @return Number of closed captions parsed on success, negative error code on failure.
 *         If no Closed Captions are parsed, *pbuf is untouched.
 */
#if 0
int ff_parse_vmeta(AVBufferRef **pbuf, const uint8_t *data, int size);
#endif
#endif /* AVCODEC_VMETA_H */
