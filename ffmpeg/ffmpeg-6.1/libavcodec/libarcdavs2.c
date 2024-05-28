
#include <unistd.h>
#include <dlfcn.h>

#include "arcdavs2.h"
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

typedef struct arcdavs2_context
{
	AVCodecContext*             avctx;
	void*                       handle;
	void*                       decoder;
	void*                       surface;
	uint8_t*                    raw_nal;
	uint8_t*                    buffer;
	int32_t                     data_len;
	int32_t                     buf_size;
	int32_t                     init;
	int32_t                     realloc;
	int32_t                     eos;
	int64_t                     pkt_pts;
	asavs2_param                param;
	asavs2_seq_info             sequence;

	PAVS2_DECODER_CREATE        avs2_decoder_create;
	PAVS2_DECODER_INIT_STREAM   avs2_decoder_init_stream;
	PAVS2_DECODER_GET_NAL       avs2_decoder_get_nal;
	PAVS2_DECODER_GET_SEQ	    avs2_decoder_get_seq;
	PAVS2_DECODER_PROBE_SEQ     avs2_decoder_probe_seq;
	PAVS2_DECODER_PIC_HEADER	avs2_decoder_pic_header;
	PAVS2_DECODE_FRAME          avs2_decode_frame;
	PAVS2_DECODER_PROCESS	    avs2_decoder_process;
	PAVS2_SEQUENCE_CHANGED      avs2_sequence_changed;
	PAVS2_DECODER_REALLOC       avs2_decoder_realloc;
	PAVS2_OUTPUT_FRAME          avs2_output_frame;
	PAVS2_UNREF_FRAME           avs2_unref_frame;
	PAVS2_WAIT_FINISH           avs2_wait_finish;
	PAVS2_DECODER_FLUSH         avs2_decoder_flush;
	PAVS2_DECODER_DESTROY       avs2_decoder_destroy;
} arcdavs2_context;

static void av_buffer_no_free(void *opaque, uint8_t *data)
{
	return;
}

static void NotifyCallback(void* ctx)
{
	return;
}

static int avs2_get_decoded_frame(AVCodecContext *avctx, AVFrame *frm)
{
	arcdavs2_context *h = avctx->priv_data;
	asavs2_param *param = &h->param;

	avctx->width = param->seq_info.lWidth;
	avctx->height = param->seq_info.lHeight;
	avctx->sample_aspect_ratio.num = 1;
	avctx->sample_aspect_ratio.den = 1;
	if (param->seq_info.profile == 0x22)
		avctx->pix_fmt = AV_PIX_FMT_YUV420P10LE;
	else
		avctx->pix_fmt = AV_PIX_FMT_YUV420P;

	frm->width = avctx->width;
	frm->height = avctx->height;
	frm->format = avctx->pix_fmt;
	frm->sample_aspect_ratio = avctx->sample_aspect_ratio;
	frm->pts = param->rtStart;
	frm->interlaced_frame = param->b_interlaced;
	frm->top_field_first = param->b_interlaced;
	frm->color_range = param->HDR_info.color_range;

	if (param->HDR_info.colour_description_present_flag)
	{
		frm->color_primaries = param->HDR_info.colour_primaries;
		frm->color_trc = param->HDR_info.transfer_characteristic;
		frm->colorspace = param->HDR_info.matrix_coeffs;
	}
	if (param->HDR_info.mastering_display_flag)
	{
		do
		{
			AVMasteringDisplayMetadata *mastering = av_mastering_display_metadata_create_side_data(frm);
			if (!mastering)
				break;

			for (int i = 0; i < 3; i++)
			{
				mastering->display_primaries[i][0] = av_make_q(param->HDR_info.display_primaries_x[i], 50000);
				mastering->display_primaries[i][1] = av_make_q(param->HDR_info.display_primaries_y[i], 50000);
			}
			mastering->white_point[0] = av_make_q(param->HDR_info.white_point_x, 50000);
			mastering->white_point[1] = av_make_q(param->HDR_info.white_point_y, 50000);

			mastering->max_luminance = av_make_q(param->HDR_info.max_display_mastering_luminance, 10000);
			mastering->min_luminance = av_make_q(param->HDR_info.min_display_mastering_luminance, 10000);

			mastering->has_primaries = 1;
			mastering->has_luminance = 1;
		} while (0);
	}
	if (param->HDR_info.content_light_level_flag)
	{
		do
		{
			AVContentLightMetadata *light = av_content_light_metadata_create_side_data(frm);
			if (!light)
				break;
			light->MaxCLL = param->HDR_info.max_content_light_level;
			light->MaxFALL = param->HDR_info.max_pic_average_light_level;
		} while (0);
	}

	for (int i = 0; i < 3; i++)
	{
		int height = i == 0 ? param->seq_info.lHeight : param->seq_info.lHeight / 2;
		int plane_size = param->u_out_stride[i] * height;
		frm->buf[i] = av_buffer_create(param->p_out_yuv[i], plane_size, av_buffer_no_free, NULL, 0);
		if (!frm->buf[i])
		{
			av_log(avctx, AV_LOG_ERROR, "Decoder error: allocation failure, can't dump frames.\n");
			return AVERROR(ENOMEM);
		}
		frm->data[i] = frm->buf[i]->data;
		frm->linesize[i] = param->u_out_stride[i];
	}

	return 0;
}

static av_cold int arcdavs2_decode_init(AVCodecContext *avctx)
{
	arcdavs2_context *h = avctx->priv_data;
	int ret;

    h->handle = dlopen("libASAVS2Vid.so", RTLD_LAZY);
    if(h->handle == NULL)
    {
        av_log(avctx, AV_LOG_ERROR, "load libASAVS2Vid.so failed: %s\n", dlerror());
        return AVERROR(EFAULT);
    }
	h->avs2_decoder_create      = (PAVS2_DECODER_CREATE)dlsym(h->handle, "asavs2_decoder_create");
	h->avs2_decoder_init_stream = (PAVS2_DECODER_INIT_STREAM)dlsym(h->handle, "asavs2_decoder_init_stream");
	h->avs2_decoder_get_nal     = (PAVS2_DECODER_GET_NAL)dlsym(h->handle, "asavs2_decoder_get_one_nal");
	h->avs2_decoder_get_seq     = (PAVS2_DECODER_GET_SEQ)dlsym(h->handle, "asavs2_decoder_get_seq");
	h->avs2_decoder_probe_seq   = (PAVS2_DECODER_PROBE_SEQ)dlsym(h->handle, "asavs2_decoder_probe_seq");
	h->avs2_decoder_pic_header  = (PAVS2_DECODER_PIC_HEADER)dlsym(h->handle, "asavs2_decoder_pic_header");
	h->avs2_decode_frame        = (PAVS2_DECODE_FRAME)dlsym(h->handle, "asavs2_decode_one_frame");
	h->avs2_decoder_process     = (PAVS2_DECODER_PROCESS)dlsym(h->handle, "asavs2_decoder_process");
	h->avs2_sequence_changed    = (PAVS2_SEQUENCE_CHANGED)dlsym(h->handle, "asavs2_video_sequence_dimension_profile_changed");
	h->avs2_decoder_realloc     = (PAVS2_DECODER_REALLOC)dlsym(h->handle, "asavs2_decoder_realloc");
	h->avs2_output_frame        = (PAVS2_OUTPUT_FRAME)dlsym(h->handle, "asavs2_output_frame");
	h->avs2_unref_frame         = (PAVS2_UNREF_FRAME)dlsym(h->handle, "asavs2_unref_output_image");
	h->avs2_wait_finish         = (PAVS2_WAIT_FINISH)dlsym(h->handle, "asavs2_wait_all_pictures_over");
	h->avs2_decoder_flush       = (PAVS2_DECODER_FLUSH)dlsym(h->handle, "asavs2_flush_decoder");
	h->avs2_decoder_destroy     = (PAVS2_DECODER_DESTROY)dlsym(h->handle, "asavs2_decoder_destroy");

    if (!h->avs2_decoder_create  || !h->avs2_decoder_init_stream || !h->avs2_decoder_get_nal ||
		!h->avs2_decoder_get_seq || !h->avs2_decoder_probe_seq   || !h->avs2_decoder_pic_header ||
		!h->avs2_decode_frame    || !h->avs2_decoder_process     || !h->avs2_sequence_changed ||
		!h->avs2_decoder_realloc || !h->avs2_output_frame        || !h->avs2_unref_frame ||
		!h->avs2_wait_finish     || !h->avs2_decoder_flush       || !h->avs2_decoder_destroy)
    {
        av_log(avctx, AV_LOG_ERROR, "get avs2 decoder api failed\n");
        return AVERROR(EFAULT);
    }

	memset(&h->param, 0, sizeof(asavs2_param));
	h->param.i_thread_num = avctx->thread_count;
	h->raw_nal = (uint8_t*)malloc(NAL_BUFFER_SIZE);
	h->buffer = (uint8_t*)malloc(NAL_BUFFER_SIZE);

    ret = h->avs2_decoder_create(&h->decoder, &h->param, NotifyCallback, h);
    if (ret != 0)
    {
        av_log(avctx, AV_LOG_ERROR, "avs3 decoder open failed\n");
        return AVERROR(EFAULT);
    }

    return 0;
}

static int parse_nalu(uint8_t *data, int len, int** nal, int* nal_size)
{
	uint8_t *p;
	uint8_t *start = data;
	uint8_t *end = data + len;
	int Code;

	while (start < end - 3)
	{
		if (start[2] == 1)
			break;
		start++;
	}
	*nal = start;
	p = start + 4;

	while (p < end - 3)
	{
		if (p[2] > 1)
		{
			p += 3;
			continue;
		}
		
		Code = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
		if (Code == CAVS_VIDEO_SEQUENCE_START_CODE || Code == CAVS_VIDEO_SEQUENCE_END_CODE ||
			Code == CAVS_INTRA_PICUTRE_START_CODE || Code == CAVS_INTER_PICUTRE_START_CODE ||
			Code == CAVS_VIDEO_EDIT_CODE)
		{
			*nal_size = p - start;
			return p - data;
		}
		p++;
	}

	*nal_size = len - (start - data);
	return len;
}

static int format_decision(arcdavs2_context* h, uint8_t* data, int size)
{
	int pkt_size = size;
	int len, ret, nals_size, raw_len;
	int start_code, have_sequence = 0;
	uint8_t* pkt_data = data;
	uint8_t* nals_data;

	while (pkt_size > 0)
	{
		len = parse_nalu(pkt_data, pkt_size, &nals_data, &nals_size);
		h->avs2_decoder_init_stream(h->decoder, nals_data, nals_size);
		start_code = h->avs2_decoder_get_nal(h->decoder, h->raw_nal, NAL_BUFFER_SIZE, &raw_len);
		
		if (CAVS_INTRA_PICUTRE_START_CODE == start_code || CAVS_INTER_PICUTRE_START_CODE == start_code)
		{
			if (have_sequence)
			{
				ret = h->avs2_decoder_pic_header(h->decoder, h->raw_nal, raw_len, &h->param, start_code);
				if (ret != CAVS_ERROR)
					return 1;
			}
		}
		else
		{
			ret = h->avs2_decoder_probe_seq(h->decoder, h->raw_nal, raw_len);
			if (ret == CAVS_SEQ_HEADER)
				have_sequence = 1;
		}
		pkt_data += len;
		pkt_size -= len;
	}

	return 0;
}

static int arcdavs2_receive_frame(AVCodecContext *avctx, AVFrame *frm)
{
    arcdavs2_context *h = avctx->priv_data;
	AVPacket avpkt;
	int len, ret = -1;
	int pkt_size, got_frame;
	int start_code, nals_size, raw_nal_len;
	uint8_t *nals_data, *pkt_data;

	av_init_packet(&avpkt);

	if (h->surface)
	{
		h->avs2_unref_frame(h->decoder, h->surface);
		h->surface = NULL;
	}

	if (!h->realloc)
	{
		ret = ff_decode_get_packet(avctx, &avpkt);
		if (ret == AVERROR_EOF)
			h->eos = 1;
	}

	if (ret >= 0 && avpkt.size)
	{
		pkt_data = avpkt.data;
		pkt_size = avpkt.size;

		if (!h->init)
		{
			h->init = format_decision(h, avpkt.data, avpkt.size);
			if (!h->init)
				return AVERROR(EAGAIN);
		}

		while (pkt_size > 0)
		{
			len = parse_nalu(pkt_data, pkt_size, &nals_data, &nals_size);
			h->avs2_decoder_init_stream(h->decoder, nals_data, nals_size);
			start_code = h->avs2_decoder_get_nal(h->decoder, h->raw_nal, NAL_BUFFER_SIZE, &raw_nal_len);

			if (start_code == CAVS_INTRA_PICUTRE_START_CODE || start_code == CAVS_INTER_PICUTRE_START_CODE)
			{
				h->param.rtStart = avpkt.pts;
				ret = h->avs2_decode_frame(h->decoder, start_code, &h->param, h->raw_nal, raw_nal_len);
			}
			else
			{
				ret = h->avs2_decoder_process(h->decoder, h->raw_nal, raw_nal_len);
				if (start_code == CAVS_VIDEO_SEQUENCE_START_CODE && raw_nal_len < nals_size)
				{
					int tot_len = raw_nal_len;
					uint8_t *data = nals_data + raw_nal_len;
					while (tot_len < nals_size)
					{
						h->avs2_decoder_init_stream(h->decoder, data, nals_size - tot_len);
						h->avs2_decoder_get_nal(h->decoder, h->raw_nal, NAL_BUFFER_SIZE, &raw_nal_len);
						h->avs2_decoder_process(h->decoder, h->raw_nal, raw_nal_len);
						data += raw_nal_len;
						tot_len += raw_nal_len;
					}
				}
			}
			if (ret == CAVS_SEQ_HEADER)
			{
				asavs2_seq_info seq;
				h->avs2_decoder_get_seq(h->decoder, &seq);
				h->param.i_color_space = CAVS_CS_YUV420;
				if (h->param.seq_header_flag == 0)
				{
					h->param.seq_header_flag = 1;
					memcpy(&h->param.seq_info, &seq, sizeof(asavs2_seq_info));
				}
				else
				{
					if (h->avs2_sequence_changed(h->decoder))
					{
						h->avs2_wait_finish(h->decoder);
						memcpy(&h->sequence, &seq, sizeof(asavs2_seq_info));
						if (h->buf_size < pkt_size - len)
						{
							h->buffer = realloc(h->buffer, pkt_size);
							h->buf_size = pkt_size;
						}
						memcpy(h->buffer, pkt_data + len, pkt_size - len);
						h->data_len = pkt_size - len;
						h->pkt_pts = avpkt.pts;
						h->realloc = 1;
						av_packet_unref(&avpkt);
						break;
					}
				}
			}
			pkt_data += len;
			pkt_size -= len;
		}
		av_packet_unref(&avpkt);
	}

	if (h->eos)
		h->avs2_wait_finish(h->decoder);

	ret = h->avs2_output_frame(h->decoder, &h->param, h->eos | h->realloc);
	if (ret == 0)
	{
		avs2_get_decoded_frame(avctx, frm);
		h->surface = h->param.h_output_image;
		return 0;
	}
	else if (h->realloc)
	{
		if (h->surface)
		{
			h->avs2_unref_frame(h->decoder, h->surface);
			h->surface = NULL;
		}

		pkt_data = h->buffer;
		pkt_size = h->data_len;
		h->avs2_decoder_realloc(h->decoder);
		memcpy(&h->param.seq_info, &h->sequence, sizeof(asavs2_seq_info));

		while (pkt_size > 0)
		{
			len = parse_nalu(pkt_data, pkt_size, &nals_data, &nals_size);
			h->avs2_decoder_init_stream(h->decoder, nals_data, nals_size);
			start_code = h->avs2_decoder_get_nal(h->decoder, h->raw_nal, NAL_BUFFER_SIZE, &raw_nal_len);

			if (start_code == CAVS_INTRA_PICUTRE_START_CODE || start_code == CAVS_INTER_PICUTRE_START_CODE)
			{
				h->param.rtStart = h->pkt_pts;
				ret = h->avs2_decode_frame(h->decoder, start_code, &h->param, h->raw_nal, raw_nal_len);
			}
			pkt_data += len;
			pkt_size -= len;
		}
		h->data_len = 0;
		h->realloc = 0;
	}
	else if (h->eos)
		return AVERROR_EOF;

	return AVERROR(EAGAIN);
}

static av_cold int arcdavs2_decode_flush(AVCodecContext *avctx)
{
	arcdavs2_context *h = avctx->priv_data;	
	h->avs2_decoder_flush(h->decoder, true);
	h->avs2_decoder_flush(h->decoder, false);

	return 0;
}

static av_cold int arcdavs2_decode_close(AVCodecContext *avctx)
{
    arcdavs2_context *h = avctx->priv_data;

	h->avs2_decoder_destroy(h->decoder);
	h->decoder = NULL;    
    dlclose(h->handle);
	h->handle = NULL;

	if (h->raw_nal)
		free(h->raw_nal);
	h->raw_nal = NULL;

	if (h->buffer)
		free(h->buffer);
	h->buffer = NULL;
	h->buf_size = 0;

    return 0;
}

const FFCodec ff_libarcdavs2_decoder = 
{
    .p.name           = "libarcdavs2",
    .p.long_name      = NULL_IF_CONFIG_SMALL("AVS2 decoder by ArcVideo"),
    .p.type           = AVMEDIA_TYPE_VIDEO,
    .p.id             = AV_CODEC_ID_AVS2,
	.p.capabilities   = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_OTHER_THREADS,
	.caps_internal    = FF_CODEC_CAP_AUTO_THREADS,
	.priv_data_size   = sizeof(arcdavs2_context),
	FF_CODEC_RECEIVE_FRAME_CB(arcdavs2_receive_frame),
    .init             = arcdavs2_decode_init,
	.flush            = arcdavs2_decode_flush,
	.close            = arcdavs2_decode_close,
};
