#include <dlfcn.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdbool.h>
#include "arcdav3a.h"
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

#define MAX_VIVID_SIZE 4096*64 //解码输出buffer 256k
#define MAX_SINT16 32768 //16进制最大值

typedef struct AVS3DecoderOutput
{
	char *pOutData;
	unsigned long nlen;	//in/out
	unsigned long nChannel;
	unsigned long nSamplerate;
	unsigned long nBits;
	unsigned long nChCfg;
	unsigned long nObjCnt;
	char *pMeta;	//alloc inside, need free outside
	unsigned long nMeta;
}AVS3DecoderOutput;


typedef struct Av3aSideData
{
    bool BinauralRender;
}Av3aSideData;

typedef struct arcdav3a_context
{
	AVCodecContext*     avctx;
	int                 eos;
	AVS3DecoderOutput   out_frame;
	bool		m_bFirstFrame;
	Avs3MetaData m_LastMetaData;
	AVS3DecoderHandle m_hAvs3;
    
	char* m_pBuffer;
	unsigned long m_dwBufferSize;
	unsigned long m_dwDataLen;

	unsigned long m_dwErrorCounter;
	int m_nlastChannels;
	int m_lastChCfg;
	int m_lastObjcnt;
	int m_lastMixtype;
	int m_lastBitrateTotal;

	void*   handle;
	void*   renderhandle;
	PFavs3_create_decoder avs3_create_decoder;
	PFavs3_destroy_decoder avs3_destroy_decoder;
	PFparse_header parse_header;
	PFavs3_decode avs3_decode;

    bool        m_bGotMD;
    void*		m_pRender;
	float* m_pRenderBuffer;
	unsigned long m_dwRenderBufferSize;
	unsigned long m_dwRenderDataLen;
	PFCreateRenderer  CreateRenderer;
	PFPutInterleavedAudioBuffer  PutInterleavedAudioBuffer;
	PFGetBinauralInterleavedAudioBuffer GetBinauralInterleavedAudioBuffer;
    PFUpdateMetadata  UpdateMetadata;
	PFSetListenerPosition  SetListenerPosition;
	PFDestroyRenderer    DestroyRenderer;
	bool BinaRender;
} arcdav3a_context;


static int  InitRender(AVCodecContext *avctx, AVS3DecoderOutput* pOut)
{
	arcdav3a_context *h = avctx->priv_data;
	h->renderhandle = dlopen("libav3a_binaural_render.so", RTLD_LAZY);
	if(h->renderhandle == NULL)
    {
        av_log(avctx, AV_LOG_ERROR, "load libav3a_binaural_render.so failed: %s\n", dlerror());
        return AVERROR(EFAULT);
    }
	h->CreateRenderer  = (PFCreateRenderer)dlsym(h->renderhandle, "CreateRenderer");
	h->PutInterleavedAudioBuffer = (PFPutInterleavedAudioBuffer)dlsym(h->renderhandle, "PutInterleavedAudioBuffer");
	h->GetBinauralInterleavedAudioBuffer = (PFGetBinauralInterleavedAudioBuffer)dlsym(h->renderhandle, "GetBinauralInterleavedAudioBuffer");
	h->UpdateMetadata = (PFUpdateMetadata)dlsym(h->renderhandle, "UpdateMetadata");
	h->SetListenerPosition = (PFSetListenerPosition)dlsym(h->renderhandle, "SetListenerPosition");
	h->DestroyRenderer = (PFDestroyRenderer)dlsym(h->renderhandle, "DestroyRenderer");


    if (h->CreateRenderer == NULL || h->PutInterleavedAudioBuffer == NULL || 
		h->GetBinauralInterleavedAudioBuffer == NULL || h->UpdateMetadata == NULL ||
	    h->SetListenerPosition == NULL || h->DestroyRenderer == NULL)
    {
        av_log(avctx, AV_LOG_ERROR, "get avs3 audio decoder api failed\n");
        return AVERROR(EFAULT);
    }

	if (!h->m_pRender)
	{
		Avs3MetaData metaData = { 0 };
		//		if (pOut->pMeta && (pOut->nMeta > 0))
		{
			memcpy(&metaData, &h->m_LastMetaData, sizeof(metaData));
		}
		h->m_pRender = h->CreateRenderer(&metaData, pOut->nSamplerate, 1024);
		av_log(avctx, AV_LOG_DEBUG, "CreateRenderer h->m_pRender:%p \n", h->m_pRender);
	}
	return 0;
}

static av_cold int arcdav3a_decode_init(AVCodecContext *avctx)
{
	av_log(avctx, AV_LOG_DEBUG, "begin arcdav3a_decode_init!\n");
	arcdav3a_context *h = avctx->priv_data;
	avctx->sample_fmt = AV_SAMPLE_FMT_S16;//for find_stream_info port to reduce time consuming
	//avctx->ch_layout  = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
	if(!h->handle)
	{
		h->handle = dlopen("libAVS3AudioDec.so", RTLD_LAZY);
	}
	
    if(h->handle == NULL)
    {
        av_log(avctx, AV_LOG_ERROR, "load libAVS3AudioDec.so failed: %s\n", dlerror());
        return AVERROR(EFAULT);
    }

	h->avs3_create_decoder  = (PFavs3_create_decoder)dlsym(h->handle, "avs3_create_decoder");
	h->avs3_destroy_decoder = (PFavs3_destroy_decoder)dlsym(h->handle, "avs3_destroy_decoder");
	h->parse_header = (PFparse_header)dlsym(h->handle, "parse_header");
	h->avs3_decode = (PFavs3_decode)dlsym(h->handle, "avs3_decode");

    if (h->avs3_create_decoder == NULL || h->avs3_destroy_decoder == NULL || 
		h->parse_header == NULL || h->avs3_decode == NULL)
    {
        av_log(avctx, AV_LOG_ERROR, "get avs3 audio decoder api failed\n");
        return AVERROR(EFAULT);
    }
	if (!h->m_hAvs3)
		h->m_hAvs3 = h->avs3_create_decoder();
	
	h->m_lastChCfg = -1;
	h->m_lastObjcnt = -1;
	h->m_lastMixtype = -1;
	h->m_lastBitrateTotal = -1;
	h->m_bFirstFrame = true;
	h->m_dwDataLen = 0;
	h->m_pBuffer = NULL;
	h->m_dwBufferSize = 0;

	memset(&h->m_LastMetaData, 0, sizeof(h->m_LastMetaData));
	memset(&h->out_frame, 0, sizeof(AVS3DecoderOutput));
	h->out_frame.pOutData = av_malloc(MAX_VIVID_SIZE*sizeof(unsigned char));
	memset(h->out_frame.pOutData, 0, MAX_VIVID_SIZE*sizeof(unsigned char));
	h->m_pRender = NULL;
	h->m_pRenderBuffer = NULL;
	h->m_bGotMD = false;
	h->renderhandle = NULL;
	h->BinaRender = false;
	av_log(avctx, AV_LOG_DEBUG, "end arcdav3a_decode_init!\n");
    return 0;
}

static av_cold void arcdav3a_decode_flush(AVCodecContext *avctx)
{
	av_log(avctx, AV_LOG_DEBUG, "begin  arcdav3a_decode_flush!\n");
	arcdav3a_context *h = avctx->priv_data;
	if (h->m_hAvs3)
		h->avs3_destroy_decoder(h->m_hAvs3);
	av_log(avctx, AV_LOG_DEBUG, "arcdav3a_decode_flush! avs3_destroy_decoder end!\n");
	h->m_hAvs3 = 0;
	h->m_hAvs3 = h->avs3_create_decoder();
	av_log(avctx, AV_LOG_DEBUG, "arcdav3a_decode_flush! avs3_create_decoder end!\n");
	h->m_bFirstFrame = true;
	h->m_dwDataLen = 0;
	h->m_lastChCfg = -1;
	h->m_lastObjcnt = -1;
	h->m_lastMixtype = -1;
	h->m_lastBitrateTotal = -1;
	av_log(avctx, AV_LOG_DEBUG, "end  arcdav3a_decode_flush!\n");
}

static int dav3a_decode_frame(AVCodecContext *avctx, const char * pIn, unsigned long lenIn, unsigned long* lenConsumed, AVS3DecoderOutput* pOut)
{
	av_log(avctx, AV_LOG_DEBUG, "begin  dav3a_decode_frame!\n");
	arcdav3a_context *h = avctx->priv_data;
    int ret = 0;
	if (!h->m_pBuffer)
	{
		h->m_dwBufferSize = lenIn * 2;
		h->m_pBuffer = av_malloc(h->m_dwBufferSize);
	}
	if (h->m_dwDataLen + lenIn > h->m_dwBufferSize)
	{
		h->m_dwBufferSize = h->m_dwDataLen + lenIn * 2;
		char* ptmp = av_realloc(h->m_pBuffer, h->m_dwBufferSize);
		if (ptmp)
			h->m_pBuffer = ptmp;
		else
		    av_free(h->m_pBuffer);
	}
	if (!h->m_pBuffer)
	{
		av_log(avctx, AV_LOG_ERROR, "av3a memory error!");
	}

	memcpy(h->m_pBuffer + h->m_dwDataLen, pIn, lenIn);
	h->m_dwDataLen += lenIn;
	*lenConsumed = lenIn;

	//unsigned long outmaxlen = pOut->nlen;
	pOut->nlen = 0;
	
	int pos = 0;
	int outindex = 0;
	av_log(avctx, AV_LOG_DEBUG, "begin  audio vivid parse_header!\n");
	do {
		int consumed = 0;
		while ((ret = h->parse_header(h->m_hAvs3, h->m_pBuffer + pos, h->m_dwDataLen - pos, h->m_bFirstFrame, &consumed, NULL)) != AVS3_TRUE)
		{
			if (ret == AVS3_DATA_NOT_ENOUGH)
			{
				if (pos + consumed < h->m_dwDataLen)
					pos += consumed;
				else
				{
					pos = h->m_dwDataLen;
				}
				break;
			}
			//printf("!!!jl: %s, parse_header return:%d, consumed:%d, pos:%d\n", __FUNCTION__, ret, consumed, pos);
			if(pos + consumed < h->m_dwDataLen)
				pos += consumed;
			else
			{
				pos = h->m_dwDataLen;
				ret = AVS3_DATA_NOT_ENOUGH;
				break;
			}
		}

		if (ret != AVS3_TRUE)
		{
			av_log(avctx, AV_LOG_DEBUG, "parse_header return:%d, pos(%d), m_dwDataLen(%ld)", ret, pos, h->m_dwDataLen);
			break;
		}

        //ret 
		if (h->m_hAvs3->numObjsOutput > 6)
		{
			av_log(avctx, AV_LOG_DEBUG, "!!!jl:Error, numObjsOutput:%d, reset\n", h->m_hAvs3->numObjsOutput);
			arcdav3a_decode_flush(avctx);
			outindex = 0;
			break;
		}

		if (((h->m_lastChCfg != -1) && (h->m_lastChCfg != h->m_hAvs3->channelNumConfig))
			|| ((h->m_lastObjcnt != -1) && (h->m_lastObjcnt != h->m_hAvs3->numObjsOutput))
			|| ( (h->m_lastMixtype != -1) && (h->m_lastMixtype != h->m_hAvs3->isMixedContent))
			|| ( (h->m_lastBitrateTotal != -1) && (h->m_lastBitrateTotal != h->m_hAvs3->totalBitrate))
			)
		{
			av_log(avctx, AV_LOG_DEBUG, "\n\nconfiguration changed, reset. m_lastChCfg:%d, now:%d, m_lastObjcnt:%d, now:%d,"
				"m_lastMixtype:%d, now:%d, m_lastBitrateTotal:%d, now:%ld\n\n"
				, h->m_lastChCfg, h->m_hAvs3->channelNumConfig, h->m_lastObjcnt, h->m_hAvs3->numObjsOutput,
				h->m_lastMixtype, h->m_hAvs3->isMixedContent, h->m_lastBitrateTotal, h->m_hAvs3->totalBitrate);
			arcdav3a_decode_flush(avctx);
			outindex = 0;
			break;
		}

		if (pos + consumed < h->m_dwDataLen)
			pos += consumed;
		else
		{
			pos = h->m_dwDataLen;
			ret = AVS3_DATA_NOT_ENOUGH;
			av_log(avctx, AV_LOG_DEBUG, "pos + consumed >= m_dwDataLen\n", pos, consumed, h->m_dwDataLen);
			break;
		}
        //reset end!
 
//		ASLOG_MsgA(5, "%s, m_dwDataLen:%d, m_hAvs3->bitsPerFrame:%d, %d", __FUNCTION__, m_dwDataLen, m_hAvs3->bitsPerFrame, m_hAvs3->bitsPerFrame/8);

		if (h->m_bFirstFrame)
			h->m_bFirstFrame = false;

		int outlen = 0;
		av_log(avctx, AV_LOG_DEBUG, "begin  avs3_decode! pos:%d m_dwDataLen:%ld h->m_pBuffer:%p pOut->pOutData:%p\n", pos, h->m_dwDataLen, h->m_pBuffer, pOut->pOutData);
		ret = h->avs3_decode(h->m_hAvs3, h->m_pBuffer + pos, h->m_dwDataLen - pos, pOut->pOutData + outindex, &outlen, &consumed);
		pos += consumed;

		if ((ret != AVS3_TRUE) || (outlen <= 0))
		{
			h->m_dwErrorCounter++;
			if (h->m_dwErrorCounter > 50)
			{
				av_log(avctx, AV_LOG_DEBUG, "avs3_decode did not output data too many times, reset decoder.");
				h->m_dwErrorCounter = 0;
				arcdav3a_decode_flush(avctx);
				outindex = 0;
			}
		}
		else
			h->m_dwErrorCounter = 0;

		if (ret != AVS3_TRUE)
			break;

		h->m_lastChCfg = h->m_hAvs3->channelNumConfig;
		h->m_lastObjcnt = h->m_hAvs3->numObjsOutput;
		h->m_lastMixtype = h->m_hAvs3->isMixedContent;
		h->m_lastBitrateTotal = h->m_hAvs3->totalBitrate;

		if (outlen)
			outindex += outlen;
		else
			break;
	} while (h->m_dwDataLen > pos);

	if (outindex)
	{
		pOut->nlen = outindex;
		pOut->nBits = 16;// m_hAvs3->bitDepth;
		pOut->nChannel = h->m_hAvs3->numChansOutput;
		pOut->nSamplerate = h->m_hAvs3->outputFs;
		pOut->nChCfg = h->m_hAvs3->channelNumConfig;
		pOut->nObjCnt = h->m_hAvs3->numObjsOutput;
		if (h->m_hAvs3->hMetadataDec->avs3MetaData.hasStaticMeta || h->m_hAvs3->hMetadataDec->avs3MetaData.hasDynamicMeta)
		{
			pOut->nMeta = sizeof(h->m_hAvs3->hMetadataDec->avs3MetaData);
			pOut->pMeta = (char*)&h->m_hAvs3->hMetadataDec->avs3MetaData;
		}
		else
		{
			pOut->nMeta = 0;
			pOut->pMeta = NULL;
		}
		h->m_hAvs3->numObjsOutput = 0;
	}

	// Adjust input blob 
	if ( (pos > 0) && (pos < h->m_dwDataLen) )
	{
		memmove(h->m_pBuffer, h->m_pBuffer + pos, h->m_dwDataLen - pos);
		h->m_dwDataLen -= pos;
	}
	else if(pos >= h->m_dwDataLen)
		h->m_dwDataLen = 0;

	return ret;
}


static int RenderPCM(AVCodecContext *avctx, AVS3DecoderOutput* pOut)
{
	av_log(avctx, AV_LOG_DEBUG, "begin  RenderPCM!\n");
	arcdav3a_context *h = avctx->priv_data;
	int nRet;
	if(!h->renderhandle){
		av_log(avctx, AV_LOG_DEBUG, "InitRender!\n");
		nRet = InitRender(avctx, &h->out_frame);
		if(nRet < 0)
            return nRet;
	}

	if (!h->renderhandle)
		return -1;

	if (!pOut || !pOut->pOutData || pOut->nlen <= 0)
		return -1;

	if (h->m_nlastChannels != pOut->nChannel)
	{
		if(h->m_pRenderBuffer)
		    av_freep(h->m_pRenderBuffer);
		h->m_dwRenderBufferSize = h->m_dwRenderDataLen = 0;
		h->m_nlastChannels = pOut->nChannel;
	}

	int samples = pOut->nlen / (pOut->nChannel * pOut->nBits / 8);//总sample个数：输出数据长度/(声道数*2) = 一个声道的sample数=1024

	/*内存分配*/
	if (!h->m_pRenderBuffer)
	{
		h->m_dwRenderBufferSize = samples * 2;//render buffer 长度 输出2声道
		h->m_pRenderBuffer = (float*)av_malloc(h->m_dwRenderBufferSize * pOut->nChannel * sizeof(float));
		h->m_dwRenderDataLen = 0;
	}
	if (h->m_dwRenderDataLen + samples > h->m_dwRenderBufferSize)
	{
		h->m_dwRenderBufferSize = h->m_dwRenderDataLen + samples * 2;
		float* tmp = (float*)av_realloc(h->m_pRenderBuffer, h->m_dwRenderBufferSize * pOut->nChannel * sizeof(float));
		if (tmp)
			h->m_pRenderBuffer = tmp;
		else
			av_freep(h->m_pRenderBuffer);
	}
	if (!h->m_pRenderBuffer)
		return -1;
    /*内存分配 结束*/

	short *src = (short*)pOut->pOutData;//decode的输出 render的输入 此处最好copy下 然后pOut->pOutData置空？
	for (int i = 0; i < samples*pOut->nChannel; i++)
	{
		*(h->m_pRenderBuffer + h->m_dwRenderDataLen * pOut->nChannel + i) = ((float)src[i]) / MAX_SINT16;
	}
	h->m_dwRenderDataLen += samples;//要render的sample个数
	Avs3MetaData metaData = { 0 };
//	if (pOut->pMeta && (pOut->nMeta > 0))
	{
//		memcpy(&metaData, pOut->pMeta, min(pOut->nMeta, sizeof(metaData)));
		memcpy(&metaData, &h->m_LastMetaData, sizeof(h->m_LastMetaData));
	}
	float position[3] = { 0, 0, 0 };
	float front[3] = { 0, 0, 1 };
	float up[3] = { 0, 1, 0 };

	float outbuf[1024 * 16];//16声道 每个声道1024个sample

	pOut->nChannel = 2;//输出2声道
	pOut->nlen = 0;//输出长度置空
	int err = 0;
	int pos = 0;
	while (h->m_dwRenderDataLen - pos >= 1024)//一次render 1024 个sample
	{
		av_log(avctx, AV_LOG_DEBUG, "begin  PutInterleavedAudioBuffer!\n");
		if (h->PutInterleavedAudioBuffer(h->m_pRender, h->m_pRenderBuffer, 1024, h->m_nlastChannels) != 0) //m_pRender: handle  m_pRenderBuffer:存放要render的sample
		{
			err = 1;
			break;
		}
//		if (pOut->pMeta && (pOut->nMeta > 0))
		{
			av_log(avctx, AV_LOG_DEBUG, "begin  UpdateMetadata!\n");
			if (h->UpdateMetadata(h->m_pRender, &metaData) != 0) //如果该帧有metadata  更新media data
			{
				err = 1;
				break;
			}
		}
		if (h->SetListenerPosition(h->m_pRender, position, front, up) != 0) {
			err = 1;
			break;
		}
		av_log(avctx, AV_LOG_DEBUG, "begin  GetBinauralInterleavedAudioBuffer!  %p\n", h->m_pRender);
		if (h->GetBinauralInterleavedAudioBuffer(h->m_pRender, outbuf, 1024) != 0) //获取输出数据 outbuf： 输出数据
		{
			err = 1;
			break;
		}
		av_log(avctx, AV_LOG_DEBUG, "end  GetBinauralInterleavedAudioBuffer!\n");
		short* dst = (short*)(pOut->pOutData + pOut->nlen);//指向输出位置 相当于覆盖了原来的输出数据的内容
		for (int i = 0; i < 1024*pOut->nChannel; i++)//此处 pOut->nChannel是2
		{
			float tmp = outbuf[i] * 32767.0f;
            if (tmp > 32767.0f) {
                tmp = 32767.0f;
            } else if (tmp < -32768.0f) {
                tmp = -32768.0f;
            }
            dst[i] = (short)tmp;//float转int
		}
		pOut->nlen += 1024 * pOut->nChannel * 2;
		pos += 1024;
	}

	if (err)
	{
		pOut->nlen = 0;
		h->m_dwRenderDataLen = 0;
	}
	if (h->m_dwRenderDataLen > pos)
		h->m_dwRenderDataLen -= pos;
	else
		h->m_dwRenderDataLen = 0;
    av_log(avctx, AV_LOG_DEBUG, "end  RenderPCM!\n");
	return 0;
}


static int arcdav3a_decode_frame(AVCodecContext *avctx, AVFrame *frm, int *got_frame_ptr, AVPacket *avpkt)
{
    av_log(avctx, AV_LOG_DEBUG, "begin arcdav3a_receive_frame!\n");
	arcdav3a_context *h = avctx->priv_data;
	int ret;
    *got_frame_ptr = 0;
	size_t side_data_size;
    uint8_t *side_data;
    side_data = av_packet_get_side_data(avpkt, AV_PKT_DATA_AUDIO_VIVID,
                                                &side_data_size);
	if(side_data && side_data_size>0){
											
        h->BinaRender = fasle;//((Av3aSideData*)side_data)->BinauralRender == true?true:false;
		av_log(avctx, AV_LOG_DEBUG, "h->BinaRender=%d\n", h->BinaRender);
	}

	//static int num =0;
	//num ++;
	//FILE* fpin = NULL;
	// FILE* fpout = NULL;
	// if (num == 1)
	// {
    //     //fpin = fopen("test.av3a", "w");
	// 	fpout = fopen("/sdcard/test.pcm", "w");
	// }
    // else
	// {
    //     //fpin = fopen("test.av3a", "a+");
	// 	fpout = fopen("/sdcard/test.pcm", "a+");
	// }
	/*if(avpkt.size > 0 && avpkt.data)
	{
		fwrite(avpkt.data, 1, avpkt.size, fpin);
	}
	fclose(fpin);*/
	
	//unsigned char pbIn[2048] = {0};
	if (avpkt->size>0)
	{
		// if(avpkt.size > 0)
		//     memcpy(pbIn, avpkt.data, avpkt.size);//此处可以不copy  dav3a_decode_frame内部会copy
		char* pbIn = avpkt->data;
		int size = avpkt->size;
		while (size > 0)//
		{
			unsigned long  lenConsumed = 0;
			memset(h->out_frame.pOutData, 0, MAX_VIVID_SIZE*sizeof(unsigned char));
			
			ret = dav3a_decode_frame(avctx, pbIn, size, &lenConsumed, &h->out_frame);
			if (ret < 0)
				return AVERROR(EAGAIN);
		
			pbIn += lenConsumed;
			size -= lenConsumed;
			//printf("!!!jl: lenConsumed:%d  size=%d h->out_frame.nlen=%d\n", lenConsumed, size, h->out_frame.nlen);
			
			if (h->out_frame.pMeta && memcmp(h->out_frame.pMeta, &h->m_LastMetaData, sizeof(h->m_LastMetaData)) != 0)//metadata发生变化
			{
				memcpy(&h->m_LastMetaData, h->out_frame.pMeta, sizeof(h->m_LastMetaData));//更新meta data
				h->m_bGotMD = true;
			}
			if(h->BinaRender && h->m_bGotMD){
                ret = RenderPCM(avctx, &h->out_frame);
				if (ret < 0)
				return AVERROR(EAGAIN);
			}


            /*if(h->out_frame.nlen > 0)
			{
				fwrite(h->out_frame.pOutData, 1, h->out_frame.nlen, fpout);
			}
			fclose(fpout);*/
			
			frm->nb_samples = 1024;
			frm->sample_rate = h->out_frame.nSamplerate;
			frm->channels = h->out_frame.nChannel;
			frm->channel_layout = av_get_default_channel_layout(h->out_frame.nChannel);
			ChannelNumConfig chconf = (ChannelNumConfig)h->out_frame.nChCfg;
			if(h->out_frame.nChannel == 1)
			    frm->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_MONO;// need set frame->ch_payout to frame ch_out verification
			else if(h->out_frame.nChannel == 2)
			    frm->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;// need set frame->ch_payout to frame ch_out verification
			else if(chconf == CHANNEL_CONFIG_MC_5_1_4 && h->out_frame.nChannel == 10)
			    frm->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_5POINT1POINT4_BACK;
			else if(chconf == CHANNEL_CONFIG_MC_7_1_2 && h->out_frame.nChannel == 10)
			    frm->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_7POINT1POINT2;
			else if(chconf == CHANNEL_CONFIG_MC_7_1_4 && h->out_frame.nChannel == 12)
			    frm->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_7POINT1POINT4_BACK;
			else if(chconf == CHANNEL_CONFIG_HOA_ORDER3 && h->out_frame.nChannel == 16)
			    frm->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_HEXADECAGONAL;
			else if(chconf == CHANNEL_CONFIG_UNKNOWN)//error
			    av_log(avctx, AV_LOG_ERROR, "unknown audio chconf! Please check the source...\n");
            avctx->ch_layout  = frm->ch_layout;//need reset avctx->ch_layout for ff_get_buffer to get correct size
			avctx->ch_layout.nb_channels = h->out_frame.nChannel;
			avctx->sample_rate = frm->sample_rate;

			frm->format = AV_SAMPLE_FMT_S16;
			av_log(avctx, AV_LOG_DEBUG, " before ff_get_buffer! h->out_frame.nlen %ld\n", h->out_frame.nlen);
			ret = ff_get_buffer(avctx, frm, 0);//will copy frm->ch_layout from avctx->ch_layout
			if (ret < 0){
				av_log(avctx, AV_LOG_DEBUG, "ff_get_buffer error!\n");
				return ret;
			}
			
			if(h->out_frame.nlen > 0)
			{
				av_log(avctx, AV_LOG_DEBUG, "begin copy buffer!\n");
				memcpy(frm->data[0], h->out_frame.pOutData, h->out_frame.nlen);
				av_log(avctx, AV_LOG_DEBUG, "end copy buffer!\n");
				/*if(fpout)
				{
				fwrite(frm->data[0], 1, h->out_frame.nlen, fpout);
				fclose(fpout); 
				}*/ 
				*got_frame_ptr = 1;                                                                                                                                                                                                                                                                                                                                        
			}
	    }
		av_log(avctx, AV_LOG_DEBUG, "av_packet_unref!\n");
		//av_packet_unref(avpkt);
		av_log(avctx, AV_LOG_DEBUG, "end av_packet_unref!\n");
	}
    //return AVERROR(EAGAIN);
	return avpkt->size;
}

static av_cold int arcdav3a_decode_close(AVCodecContext *avctx)
{
    av_log(avctx, AV_LOG_DEBUG, "arcdav3a_decode_close begin!\n");
	arcdav3a_context *h = avctx->priv_data;
	if(h == NULL)
	{
	    av_log(avctx, AV_LOG_DEBUG, "arcdav3a_context is NULL!\n");
		return 0;
	}
	if (h->m_hAvs3)
		h->avs3_destroy_decoder(h->m_hAvs3);
	av_log(avctx, AV_LOG_DEBUG, "avs3_destroy_decoder end!\n");
	if (h->m_pRender)
        h->DestroyRenderer(h->m_pRender);
	av_log(avctx, AV_LOG_DEBUG, "DestroyRenderer end!\n");
	h->m_pRender = NULL;
	if (h->renderhandle)
	    dlclose(h->renderhandle);
	av_log(avctx, AV_LOG_DEBUG, "renderhandle close end!\n");
	h->renderhandle = NULL;
    h->m_hAvs3 = NULL;
	if(h->handle)
	    dlclose(h->handle);
	av_log(avctx, AV_LOG_DEBUG, "handle close end!\n");
	h->handle = NULL;
	if(h->m_pBuffer)
	    av_freep(&h->m_pBuffer);
	av_log(avctx, AV_LOG_DEBUG, "free h->m_pBuffer end!\n");
	if(h->out_frame.pOutData)
	    av_freep(&h->out_frame.pOutData);
	av_log(avctx, AV_LOG_DEBUG, "free out_frame.pOutData end!\n");
	if(h->m_pRenderBuffer)
	    av_freep(&h->m_pRenderBuffer);
	av_log(avctx, AV_LOG_DEBUG, "arcdav3a_decode_close end!\n");
    return 0;
}
 

const FFCodec ff_libarcdav3a_decoder = 
{
    .p.name           = "libarcdav3a",
    .p.long_name      = NULL_IF_CONFIG_SMALL("AV3a decoder by ArcVideo"),
    .p.type           = AVMEDIA_TYPE_AUDIO,
    .p.id             = AV_CODEC_ID_AV3A,
	.p.capabilities   = AV_CODEC_CAP_CHANNEL_CONF | AV_CODEC_CAP_DR1,
	.caps_internal    = FF_CODEC_CAP_INIT_CLEANUP,
	.priv_data_size   = sizeof(arcdav3a_context),
	FF_CODEC_DECODE_CB(arcdav3a_decode_frame),
    .init             = arcdav3a_decode_init,
	.flush            = arcdav3a_decode_flush,
	.close            = arcdav3a_decode_close,
};
