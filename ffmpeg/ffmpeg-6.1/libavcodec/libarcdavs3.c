
#include <unistd.h>
#include <dlfcn.h>

#include "arcdavs3.h"
#include "decode.h"
#include "avcodec.h"
#include "internal.h"
#include "codec_internal.h"
#include "libavutil/mastering_display_metadata.h"
#if ARCH_AARCH64
#include <arm_neon.h>
#elif ARCH_X86
#include <emmintrin.h>
#endif

typedef struct arcdavs3_context
{
	AVCodecContext*             avctx;
	void*                       handle;
	void*                       decoder;
	void*                       surface;
	int                         eos;
	davs3_seq_info_t            headerset;
	davs3_picture_t             out_frame;
	PAVS3_DECODER_OPEN          davs3_decoder_open;
	PAVS3_DECODER_SEND_PACKET   davs3_decoder_send_packet;
	PAVS3_DECODER_RECV_FRAME    davs3_decoder_recv_frame;
	PAVS3_DECODER_FLUSH         davs3_decoder_flush;
	PAVS3_DECODER_RECYCLE_FRAME davs3_decoder_recycle_frame;
	PAVS3_DECODER_RESET         davs3_decoder_reset;
	PAVS3_DECODER_CLOSE         davs3_decoder_close;
} arcdavs3_context;

static void av_buffer_no_free(void *opaque, uint8_t *data)
{
	return;
}

static void arcdavs3_log_callback(int loglevel, char *psz_fmt)
{
	int level = 0;

	switch (loglevel)
	{
	case 1:
	case 2:
		level = AV_LOG_ERROR;
		break;
	case 3:
		level = AV_LOG_WARNING;
		break;
	case 4:
	case 5:
		level = AV_LOG_INFO;
		break;
	case 6:
		level = AV_LOG_DEBUG;
		break;
	default:
		level = AV_LOG_QUIET;
		break;
	}
	av_log(NULL, level, "%s", psz_fmt);
}

static uint8_t* find_start_code(uint8_t *data, int len, int* nal_size)
{
	uint8_t *p = data + 3;
	uint8_t *end = data + len;
	
	while (p < end - 3)
	{
		if (p[2] > 1)
		{
			p += 3;
			continue;
		}
		if (p[0] == 0 && p[1] == 0 && p[2] == 1)
		{
			*nal_size = p - data;
			return p;
		}
		p++;
	}

	*nal_size = len;
	return NULL;
}

static int convert_yuv420p16_to_yv12(uint8_t* dst[], uint8_t* src[], int width, int height, int srcStride, int dstStride)
{
	int h, w;
	uint8_t *pSrcY = src[0];
	uint8_t *pSrcU = src[1];
	uint8_t *pSrcV = src[2];
	uint8_t *pDstY = dst[0];
	uint8_t *pDstU = dst[1];
	uint8_t *pDstV = dst[2];

#if ARCH_AARCH64
	uint8x16_t S0, S1, D0;

	for (h = 0; h < height; h++)
	{
		for (w = 0; w < width - 15; w += 16)
		{
			S0 = vld1q_u8(pSrcY + 2 * w);
			S1 = vld1q_u8(pSrcY + 2 * 2 + 16);
			D0 = vuzp1q_u8(S0, S1);
			vst1q_u8(pDstY, D0);
		}
		for (; w < width; w++)
		{
			pDstY[w] = pSrcY[2 * w];
		}
		pSrcY += srcStride;
		pDstY += dstStride;
	}

	width >>= 1;
	height >>= 1;
	srcStride >>= 1;
	dstStride >>= 1;
	for (h = 0; h < height; h++)
	{
		for (w = 0; w < width - 15; w += 16)
		{
			S0 = vld1q_u8(pSrcU + 2 * w);
			S1 = vld1q_u8(pSrcU + 2 * w + 16);
			D0 = vuzp1q_u8(S0, S1);
			vst1q_u8(pDstY, D0);
		}
		for (; w < width; w++)
		{
			pDstU[w] = pSrcU[2 * w];
		}
		pSrcU += srcStride;
		pDstU += dstStride;
	}

	for (h = 0; h < height; h++)
	{
		for (w = 0; w < width - 15; w += 16)
		{
			S0 = vld1q_u8(pSrcV + 2 * w);
			S1 = vld1q_u8(pSrcV + 2 * w + 16);
			D0 = vuzp1q_u8(S0, S1);
			vst1q_u8(pDstV + w, D0);
		}
		for (; w < width; w++)
		{
			pDstV[w] = pSrcV[2 * w];
		}
		pSrcV += srcStride;
		pDstV += dstStride;
	}
#elif ARCH_X86
	__m128i S0, S1, D0;

	for (h = 0; h < height; h++)
	{
		for (w = 0; w < width - 15; w += 16)
		{
			S0 = _mm_loadu_si128((__m128i*)(pSrcY + 2 * w));
			S1 = _mm_loadu_si128((__m128i*)(pSrcY + 2 * w + 16));
			D0 = _mm_packus_epi16(S0, S1);
			_mm_storeu_si128((__m128i*)(pDstY + w), D0);
		}
		for (; w < width; w++)
		{
			pDstY[w] = pSrcY[2 * w];
		}
		pSrcY += srcStride;
		pDstY += dstStride;
	}

	width >>= 1;
	height >>= 1;
	srcStride >>= 1;
	dstStride >>= 1;
	for (h = 0; h < height; h++)
	{
		for (w = 0; w < width - 15; w += 16)
		{
			S0 = _mm_loadu_si128((__m128i*)(pSrcU + 2 * w));
			S1 = _mm_loadu_si128((__m128i*)(pSrcU + 2 * w + 16));
			D0 = _mm_packus_epi16(S0, S1);
			_mm_storeu_si128((__m128i*)(pDstU + w), D0);
		}
		for (; w < width; w++)
		{
			pDstU[w] = pSrcU[2 * w];
		}
		pSrcU += srcStride;
		pDstU += dstStride;
	}

	for (h = 0; h < height; h++)
	{
		for (w = 0; w < width - 15; w += 16)
		{
			S0 = _mm_loadu_si128((__m128i*)(pSrcV + 2 * w));
			S1 = _mm_loadu_si128((__m128i*)(pSrcV + 2 * w + 16));
			D0 = _mm_packus_epi16(S0, S1);
			_mm_storeu_si128((__m128i*)(pDstV + w), D0);
		}
		for (; w < width; w++)
		{
			pDstV[w] = pSrcV[2 * w];
		}
		pSrcV += srcStride;
		pDstV += dstStride;
	}
#else
    for (h = 0; h < height; h++)
    {
    	for (w = 0; w < width; w++)
			pDstY[w] = pSrcY[2 * w];
    	pSrcY += srcStride;
		pDstY += dstStride;
    }

	width >>= 1;
	height >>= 1;
	srcStride >>= 1;
	dstStride >>= 1;
	for (h = 0; h < height; h++)
	{
		for (w = 0; w < width; w++)
			pDstU[w] = pSrcU[2 * w];
		pSrcU += srcStride;
		pDstU += dstStride;
	}
	for (h = 0; h < height; h++)
	{
		for (w = 0; w < width; w++)
			pDstV[w] = pSrcV[2 * w];
		pSrcV += srcStride;
		pDstV += dstStride;
	}
#endif

	return 0;
}

static int davs3_get_decoded_frame(AVCodecContext *avctx, AVFrame *frm)
{
	arcdavs3_context *h = avctx->priv_data;
	davs3_seq_info_t *headerset = &h->headerset;
	davs3_picture_t  *out_frame = &h->out_frame;
	int ret;
	int need_csconv = (headerset->output_bit_depth == 8 && headerset->bytes_per_sample == 2);

	avctx->width = headerset->width;
	avctx->height = headerset->height;
	avctx->sample_aspect_ratio.num = 1;
	avctx->sample_aspect_ratio.den = 1;
	if (headerset->chroma_format == 1 && headerset->output_bit_depth == 8)
		avctx->pix_fmt = AV_PIX_FMT_YUV420P;
	else if (headerset->chroma_format == 1 && headerset->output_bit_depth == 10)
		avctx->pix_fmt = AV_PIX_FMT_YUV420P10LE;

	if (need_csconv)
	{
		ret = ff_get_buffer(avctx, frm, 0);
		if (ret < 0)
			return ret;
	}

	frm->width = avctx->width;
	frm->height = avctx->height;
	frm->pts = out_frame->pts;
	frm->pkt_dts = out_frame->dts;
	frm->format = avctx->pix_fmt;
	frm->sample_aspect_ratio = avctx->sample_aspect_ratio;
	frm->key_frame = out_frame->type == 1;
	frm->interlaced_frame = 0;
	frm->top_field_first = 0;

	if (headerset->display.is_valid)
	{
		frm->color_range = headerset->display.sample_range + 1;
		if (headerset->display.color_description)
		{
			frm->color_primaries = headerset->display.color_primaries;
			frm->color_trc = headerset->display.transfer_characteristics;
			frm->colorspace = headerset->display.matrix_coefficients;
			switch (frm->color_trc)
			{
			case 6:
				frm->color_trc = 1;
				break;
			case 11:
				frm->color_trc = 15;
				break;
			case 12:
				frm->color_trc = 16;
				break;
			case 14:
				frm->color_trc = 18;
				break;
			}
			switch (frm->colorspace)
			{
			case 8:
				frm->colorspace = 9;
				break;
			case 9:
				frm->colorspace = 10;
				break;
			}
		}
	}
	if (headerset->mastering_display.is_valid)
	{
		do
		{
			AVMasteringDisplayMetadata *mastering = av_mastering_display_metadata_create_side_data(frm);
			if (!mastering)
				break;

			for (int i = 0; i < 3; i++)
			{
				mastering->display_primaries[i][0] = av_make_q(headerset->mastering_display.display_primaries_x[i], 50000);
				mastering->display_primaries[i][1] = av_make_q(headerset->mastering_display.display_primaries_y[i], 50000);
			}
			mastering->white_point[0] = av_make_q(headerset->mastering_display.white_point_x, 50000);
			mastering->white_point[1] = av_make_q(headerset->mastering_display.white_point_y, 50000);

			mastering->max_luminance = av_make_q(headerset->mastering_display.max_display_mastering_luminance * 10000, 10000);
			mastering->min_luminance = av_make_q(headerset->mastering_display.min_display_mastering_luminance, 10000);

			mastering->has_primaries = 1;
			mastering->has_luminance = 1;
		} while (0);

		do
		{
			AVContentLightMetadata *light = av_content_light_metadata_create_side_data(frm);
			if (!light)
				break;
			light->MaxCLL = headerset->mastering_display.max_content_light_level;
			light->MaxFALL = headerset->mastering_display.max_picture_average_light_level;
		} while (0);
	}

	if (need_csconv)
	{
		convert_yuv420p16_to_yv12(frm->data, out_frame->planes, frm->width, frm->height, out_frame->stride[0], frm->linesize[0]);
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			int plane_size = out_frame->stride[i] * out_frame->height[i];
			frm->buf[i] = av_buffer_create(out_frame->planes[i], plane_size, av_buffer_no_free, NULL, 0);
			if (!frm->buf[i])
			{
				av_log(avctx, AV_LOG_ERROR, "Decoder error: allocation failure, can't dump frames.\n");
				return AVERROR(ENOMEM);
			}
			frm->data[i] = frm->buf[i]->data;
			frm->linesize[i] = out_frame->stride[i];
		}
	}

	return 0;
}

static av_cold int arcdavs3_decode_init(AVCodecContext *avctx)
{
    arcdavs3_context *h = avctx->priv_data;

    h->handle = dlopen("libdavs3.so", RTLD_LAZY);
    if(h->handle == NULL)
    {
        av_log(avctx, AV_LOG_ERROR, "load libdavs3.so failed: %s\n", dlerror());
        return AVERROR(EFAULT);
    }

	h->davs3_decoder_open          = (PAVS3_DECODER_OPEN)dlsym(h->handle, "davs3_decoder_open");
	h->davs3_decoder_send_packet   = (PAVS3_DECODER_SEND_PACKET)dlsym(h->handle, "davs3_decoder_send_packet");
	h->davs3_decoder_recv_frame    = (PAVS3_DECODER_RECV_FRAME)dlsym(h->handle, "davs3_decoder_recv_frame");
	h->davs3_decoder_recycle_frame = (PAVS3_DECODER_RECYCLE_FRAME)dlsym(h->handle, "davs3_decoder_recycle_frame");
	h->davs3_decoder_flush         = (PAVS3_DECODER_FLUSH)dlsym(h->handle, "davs3_decoder_flush");
	h->davs3_decoder_reset         = (PAVS3_DECODER_RESET)dlsym(h->handle, "davs3_decoder_reset");
	h->davs3_decoder_close         = (PAVS3_DECODER_CLOSE)dlsym(h->handle, "davs3_decoder_close");
    if (h->davs3_decoder_open == NULL || h->davs3_decoder_send_packet == NULL || 
		h->davs3_decoder_recv_frame == NULL || h->davs3_decoder_recycle_frame == NULL ||
		h->davs3_decoder_flush == NULL || h->davs3_decoder_reset == NULL || h->davs3_decoder_close == NULL)
    {
        av_log(avctx, AV_LOG_ERROR, "get avs3 decoder api failed\n");
        return AVERROR(EFAULT);
    }

	davs3_param_t param;
	memset(&param, 0, sizeof(davs3_param_t));
	param.threads = avctx->thread_count;
	param.info_level = 6;
	param.opaque = (void*)arcdavs3_log_callback;
    h->decoder = h->davs3_decoder_open(&param);
    if (h->decoder == 0)
    {
        av_log(avctx, AV_LOG_ERROR, "avs3 decoder open failed\n");
        return AVERROR(EFAULT);
    }

    return 0;
}

static int arcdavs3_receive_frame(AVCodecContext *avctx, AVFrame *frm)
{
    arcdavs3_context *h = avctx->priv_data;
	AVPacket avpkt;
	davs3_packet_t avs3_pkt;
	int ret, got_frame;
	av_init_packet(&avpkt);

	if (h->surface)
	{
		h->davs3_decoder_recycle_frame(h->decoder, h->surface);
		h->surface = NULL;
	}

	ret = ff_decode_get_packet(avctx, &avpkt);
	if (ret == AVERROR_EOF)
		h->eos = 1;

	if (ret >= 0 && avpkt.size)
	{
		uint8_t *p_start = avpkt.data;
		uint8_t *p_next;
		int size = avpkt.size;
		int nal_size = 0;

		while (size > 0)
		{
			p_next = find_start_code(p_start, size, &nal_size);
			avs3_pkt.data = p_start;
			avs3_pkt.len = nal_size;
			avs3_pkt.pts = avpkt.pts;
			avs3_pkt.dts = avpkt.dts;

			ret = h->davs3_decoder_send_packet(h->decoder, &avs3_pkt);
			if (ret == DAVS3_ERROR)
				av_log(avctx, AV_LOG_ERROR, "An decoder error counted\n");

			p_start = p_next;
			size -= nal_size;
		}
		av_packet_unref(&avpkt);
	}
	memset(&h->out_frame, 0, sizeof(davs3_picture_t));
	got_frame = h->davs3_decoder_recv_frame(h->decoder, &h->headerset, &h->out_frame);
	if (got_frame & DAVS3_GOT_FRAME)
	{
		if (h->headerset.chroma_format == 1 && h->headerset.output_bit_depth == 10)
		{
			ret = davs3_get_decoded_frame(avctx, frm);
			if (ret < 0)
			{
				h->davs3_decoder_recycle_frame(h->decoder, &h->out_frame);
				h->surface = NULL;
				return AVERROR(EAGAIN);
			}
			h->surface = &h->out_frame;
			return 0;
		}
		else if (h->headerset.chroma_format == 1 && h->headerset.output_bit_depth == 8 && 
			     h->headerset.bytes_per_sample == 2)
		{
			ret = davs3_get_decoded_frame(avctx, frm);
			h->davs3_decoder_recycle_frame(h->decoder, &h->out_frame);
			h->surface = NULL;
			if (ret < 0)
				return AVERROR(EAGAIN);;
		}
		else
		{
			h->davs3_decoder_recycle_frame(h->decoder, &h->out_frame);
			h->surface = NULL;
			av_log(avctx, AV_LOG_ERROR, "chroma format not support\n");
			return AVERROR(EAGAIN);
		}
	}
	else if (h->eos)
		return AVERROR_EOF;

	return AVERROR(EAGAIN);
}

static av_cold int arcdavs3_decode_flush(AVCodecContext *avctx)
{
	arcdavs3_context *h = avctx->priv_data;	
	h->davs3_decoder_reset(h->decoder);

	return 0;
}

static av_cold int arcdavs3_decode_close(AVCodecContext *avctx)
{
    arcdavs3_context *h = avctx->priv_data;

	h->davs3_decoder_close(h->decoder);
	h->decoder = NULL;    
    dlclose(h->handle);
	h->handle = NULL;

    return 0;
}

const FFCodec ff_libarcdavs3_decoder = 
{
    .p.name           = "libarcdavs3",
    .p.long_name      = NULL_IF_CONFIG_SMALL("AVS3 decoder by ArcVideo"),
    .p.type           = AVMEDIA_TYPE_VIDEO,
    .p.id             = AV_CODEC_ID_AVS3,
	.p.capabilities   = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_OTHER_THREADS,
	.caps_internal    = FF_CODEC_CAP_AUTO_THREADS,
	.priv_data_size   = sizeof(arcdavs3_context),
	FF_CODEC_RECEIVE_FRAME_CB(arcdavs3_receive_frame),
    .init             = arcdavs3_decode_init,
	.flush            = arcdavs3_decode_flush,
	.close            = arcdavs3_decode_close,
};
