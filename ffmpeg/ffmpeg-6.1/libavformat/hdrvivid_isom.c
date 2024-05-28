/*
 *
 * Copyright (c) 2023 UWA(UHD World Association)
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


#include "libavcodec/put_bits.h"

#include "avformat.h"
#include "hdrvivid_isom.h"

int ff_isom_parse_cuvv(AVFormatContext *s, AVStream *st, const uint16_t *buf_ptr, uint64_t size)
{
    uint32_t buf;
    AVHDRVIVIDDecoderCUVVRecord *vivid;
    size_t vivid_size;
    int ret;

    if (size > (1 << 30) || size < 4)
        return AVERROR_INVALIDDATA;
/*
    vivid = av_hdrvivid_alloc(&vivid_size);
    if (!vivid)
        return AVERROR(ENOMEM);
*/
    vivid->cuva_version_map = *buf_ptr++;                  // 16 bits
    vivid->terminal_provide_code = *buf_ptr++;             // 16 bits
    vivid->terminal_provide_oriented_code = *buf_ptr++;    // 16 bits

    ret = av_stream_add_side_data(st, AV_PKT_DATA_VIVID_CUVV,
                                  (uint8_t *)vivid, vivid_size);
    if (ret < 0) {
        av_free(vivid);
        return ret;
    }

    av_log(s, AV_LOG_DEBUG, "HDR Vivid cuvv box, cuva_version_map: %d, terminal_provide_code: %d, terminal_provide_oriented_code: %d\n",
           vivid->cuva_version_map,vivid->terminal_provide_code,vivid->terminal_provide_oriented_code);
    return 0;
}

void ff_isom_put_cuvv(AVFormatContext *s, uint8_t out[ISOM_HDRVIVID_CUVV_SIZE],
                           AVHDRVIVIDDecoderCUVVRecord *vivid)
{
    PutBitContext pb;

    init_put_bits(&pb, out, ISOM_HDRVIVID_CUVV_SIZE);
    put_bits(&pb, 16, vivid->cuva_version_map);
    put_bits(&pb, 16, vivid->terminal_provide_code);
    put_bits(&pb, 16, vivid->terminal_provide_oriented_code);
    put_bits32(&pb, 0); /* reserved */
    put_bits32(&pb, 0); /* reserved */
    put_bits32(&pb, 0); /* reserved */
    put_bits32(&pb, 0); /* reserved */

    flush_put_bits(&pb);
    av_log(s, AV_LOG_DEBUG, "HDR Vivid cuvv box, cuva_version_map: %d, terminal_provide_code: %d, terminal_provide_oriented_code: %d\n",
           vivid->cuva_version_map,vivid->terminal_provide_code,vivid->terminal_provide_oriented_code);
}
