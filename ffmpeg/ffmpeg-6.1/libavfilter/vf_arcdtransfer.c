/*
 * Copyright (c) 2015 Paul B Mahol
 * Copyright (c) 2023 arcvideo
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
 * arcvideo HDR cuva filter using libArcHDRCUVA.so
 */

#include <float.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "CUVAHDRProcess.h"

#include "avfilter.h"
#include "formats.h"
#include "internal.h"
#include "video.h"
#include "libavutil/avstring.h"
#include "libavutil/eval.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/parseutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/hdr_dynamic_vivid_metadata.h"




#define IMAGE_CSP_MASK                      0xFF
#define IMAGE_CSP_A                         0x100
#define IMAGE_CSP_BITMASK                   0xF000
#define IMAGE_CSP_BITSHIFT					12
#define IMAGE_CSP_8BIT                      0
#define IMAGE_CSP_10BIT                     0x1000
#define IMAGE_CSP_BIT_NUM					2
#define IMAGE_CSP_MODEMASK                  0x30000
#define IMAGE_CSP_MODESHIFT					16
#define IMAGE_CSP_COM		                0
#define IMAGE_CSP_2SI		                0x10000
#define IMAGE_CSP_SQD		                0x20000

#define IMAGE_CSP_NONE                      -1
#define IMAGE_CSP_NV12                      0
#define	IMAGE_CSP_I420                      1
#define	IMAGE_CSP_YV12                      2
#define	IMAGE_CSP_YUY2                      3
#define	IMAGE_CSP_YVYU                      4
#define	IMAGE_CSP_UYVY                      5
#define	IMAGE_CSP_RGB32                     6
#define	IMAGE_CSP_RGB24                     7
#define	IMAGE_CSP_RGB565                    8
#define	IMAGE_CSP_RGB555                    9
#define IMAGE_CSP_YUV422                    10
#define IMAGE_CSP_YUV444 					11 //packet
#define IMAGE_CSP_YUV444F 					12 //planar
#define IMAGE_CSP_UYVYZ  					13 //UYVY bit zip 
#define IMAGE_CSP_NUM						14

#define IMAGE_CSP_AYUV						(IMAGE_CSP_A | IMAGE_CSP_YUV444)
#define IMAGE_CSP_ARGB32					(IMAGE_CSP_A | IMAGE_CSP_RGB32)
#define IMAGE_CSP_NV12P10BIT				(IMAGE_CSP_10BIT | IMAGE_CSP_NV12) //Most Significant 10 bits
#define IMAGE_CSP_YUV420P10BIT				(IMAGE_CSP_10BIT | IMAGE_CSP_I420)
#define IMAGE_CSP_UYVYP10BIT				(IMAGE_CSP_10BIT | IMAGE_CSP_UYVY)
#define IMAGE_CSP_YUV422P10BIT				(IMAGE_CSP_10BIT | IMAGE_CSP_YUV422)
#define IMAGE_CSP_YUV444P10BITF				(IMAGE_CSP_10BIT | IMAGE_CSP_YUV444F)
#define IMAGE_CSP_V210 						(IMAGE_CSP_10BIT | IMAGE_CSP_UYVYZ) //UYVYP10BITPacket
#define IMAGE_CSP_V211 						(IMAGE_CSP_2SI | IMAGE_CSP_V210) //UYVYP10BITPacket 2SI mode
#define IMAGE_CSP_V212						(IMAGE_CSP_SQD | IMAGE_CSP_V210) //UYVYP10BITPacket SQD mode

#define COLORPRIM_BT709		1
#define COLORPRIM_BT2020	9
#define COLORPRIM_SGAMUT	102
#define COLORPRIM_SGAMUT3   103
#define COLORPRIM_SGAMUT3CINE   104

#define TRANSFER_SDR 			1
#define TRANSFER_HLG10 			18
#define TRANSFER_HDR10			16
#define TRANSFER_SLOG2			102
#define TRANSFER_SLOG3			103

//extern  int  parserLine2MetaFrame(void* pHandle,CUVA_metadata*pMeta,const char* line);

static int vivid_meda_dump(int framenum,  AVDynamicHDRVivid *pMeta, char *metafile, char* vivid)
{
    //ofstream out_specCombine;
    AVHDRVividColorTransformParams *pVidiParam = (AVHDRVividColorTransformParams *)&pMeta->params[0];
    //char buf[64][1024] = { 0 };
    FILE *fp = NULL;
    char stemp[256]={0};
    char sline[256]={0};
    int  param_cnt = 0;
    if (metafile != NULL)
    {
        if (framenum == 1)
        fp = fopen(metafile, "w");
    else
        fp = fopen(metafile, "a+");
    }
    
    unsigned int frame_num_int = (unsigned int)framenum;
    sprintf(stemp, "%d %d %d %d %d %d %d", frame_num_int,
            pMeta->system_start_code,
            pVidiParam->minimum_maxrgb.num,
            //framenum-1,
            pVidiParam->average_maxrgb.num,pVidiParam->variance_maxrgb.num,pVidiParam->maximum_maxrgb.num,pVidiParam->tone_mapping_mode_flag);
    // out_specCombine << frame_num_int << " " << pMeta->system_start_code;
    // out_specCombine << " " << pVidiParam->minimum_maxrgb.num;
    // out_specCombine << " " << pVidiParam->average_maxrgb.num;
    // out_specCombine << " " << pVidiParam->variance_maxrgb.num;
    // out_specCombine << " " << pVidiParam->maximum_maxrgb.num;

    // out_specCombine << " " << pVidiParam->tone_mapping_mode_flag;
    if (pVidiParam->tone_mapping_mode_flag)
    {
       // out_specCombine << " " << (pVidiParam->tone_mapping_param_num - 1);
        sprintf(stemp,"%s %d",stemp,pVidiParam->tone_mapping_param_num-1);
        for (unsigned int mode_i = 0; mode_i < pVidiParam->tone_mapping_param_num; mode_i++)
        {
            sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].targeted_system_display_maximum_luminance);
            sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_enable_flag);
            if (pVidiParam->tm_params[mode_i].base_enable_flag)
            {
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_m_p.num);
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_m_m.num);
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_m_a.num);
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_m_b.num);
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_m_n.num);
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_k1);
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_k2);
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_k3);
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_Delta_enable_mode);
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].base_param_Delta);
            }
            sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].three_Spline_enable_flag);
            if (pVidiParam->tm_params[mode_i].three_Spline_enable_flag)
            {
                sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].three_Spline_num- 1);

                for (unsigned int mode_j = 0; mode_j < pVidiParam->tm_params[mode_i].three_Spline_num; mode_j++)
                {
                    sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].three_spline[mode_j].th_mode);

                    if ( pVidiParam->tm_params[mode_i].three_spline[mode_j].th_mode == 0|| pVidiParam->tm_params[mode_i].three_spline[mode_j].th_mode== 2)
                    {
                        sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].three_spline[mode_j].th_enable_mb.num);
                    }
                    sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].three_spline[mode_j].th_enable.num);
                    sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].three_spline[mode_j].th_delta1.num);
                    sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].three_spline[mode_j].th_delta2.num);
                    sprintf(stemp,"%s %d",stemp,pVidiParam->tm_params[mode_i].three_spline[mode_j].enable_strength.num);
                }
            }
        }
    }
    else{
        return -1;
    }

    //out_specCombine << " " <<  pVidiParam->color_saturation_mapping_flag;// metadata.color_saturation_mapping_flag;
    sprintf(stemp,"%s %d",stemp,pVidiParam->color_saturation_mapping_flag);
    if (pVidiParam->color_saturation_mapping_flag)
    {
        sprintf(stemp,"%s %d",stemp,pVidiParam->color_saturation_num);
        for (unsigned int mode_i = 0; mode_i < pVidiParam->color_saturation_num; mode_i++)
        {
            sprintf(stemp,"%s %d",stemp,pVidiParam->color_saturation_gain[mode_i].num);
        }
    }
    else{
        return -1;
    }

    sprintf(stemp,"%s",stemp);//这个函数重新fengzhuang下 对接yuv转yuv  此处不用\r\n
    strcpy(vivid, stemp);
    if(fp)
    {
        fprintf(fp,"%s\r\n",stemp);//此处不写文件   
        fclose(fp);
        fp = NULL;
    }
    return 0;
}

enum Transfer {
    TS_UNSPECIFIED = -1,
    TS_PQ,
    TS_HLG,
    TS_SDR,
    TS_NB
};

typedef struct ArcdTransferContext {
    const AVClass *class;
    enum MODE process_mode;
    enum AVPixelFormat pixfmt, user_format;
    int target_display_light;

    int src_transfer;//0: pQ  1: HLG  2:SDR
    enum Transfer dst_transfer;

    int nHDRType;
    int nRange;
    int bHlgModeOn;
    int sampleRange;
    int srccolorspace;
    int dstcolorspace;
    int src_color_prim;//0:auto 1:BT709, 9:BT2020
    int dst_color_prim;
    int src_maxluma;//亮度
    int src_minluma;
    int dst_maxluma;
    int dst_minluma;
    double threshold;//整体亮度， 区间为8.0 - 14.0, 默认数值为：11.0;
    double mpset;//中灰亮度:  区间为2.0 - 8.0, 默认数值为：5.0;
    int deviceid;
    int highpoint_mode;//highpoint_mode :0 HISRGB; 1:HISMAX; 2:MAXSOURCE  //is a kind of mode,高光参数 高光模式
    int cuvainit;
    stPicture *pInputPicture;
    stPicture *pOutPicture;
    void *pHandle;

    void* libHandle;
    PCUVAHDR_OPEN_PROCESSER3 OpenProcessor;
    PCUVAHDR_OUTPUT_HEADER  outputHeader;
    PCUVAHDR_CREATE_CUVA_PICTURE createCUVAPicture;
    PCUVAHDR_PARSELINE2_META_FRAME parserLine2MetaFrame;
    PCUVAHDR_POST_PROCESS_FRAME  PostProcessOneFrame;
    PCUDAHDR_OUTPUT_FOOTER  outputFooter;
    PCUVAHDR_CLOSE_PROCESSER  CloseProcessor;
    PCUVAHDR_DESTORY_CUVAPicture destoryCUVAPicture;
    
} ArcdTransferContext;

#define OFFSET(x) offsetof(ArcdTransferContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM | AV_OPT_FLAG_VIDEO_PARAM
#define TFLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_RUNTIME_PARAM

#define ENUM(x, y, z) { x, "", 0, AV_OPT_TYPE_CONST, { .i64 = y }, INT_MIN, INT_MAX, FLAGS, z }

static const AVOption arcdtransfer_options[] = {
    /*{ "mode",        "0:metadata,1:hdr,2:sdr,3:lut,4:curve,5",
      OFFSET(process_mode),   AV_OPT_TYPE_INT, { .i64 = CS_UNSPECIFIED },
      CS_UNSPECIFIED, CS_NB - 1, FLAGS,"all" },
    ENUM("metadata",   CS_META_DATA,   "all"),
    ENUM("hdr",        CS_HDR,         "all"),
    ENUM("sdr",        CS_SDR,         "all"),
    ENUM("lut",        CS_LUT,         "all"),
    ENUM("curve",      CS_CURVE,       "all"),*/

    { "transfer",       "0: pQ  1: HLG  2:SDR",
      OFFSET(dst_transfer),   AV_OPT_TYPE_INT, { .i64 = TS_UNSPECIFIED },
      TS_UNSPECIFIED, TS_NB - 1, FLAGS,"all" },
    ENUM("pq",         TS_PQ,   "all"),
    ENUM("hlg",        TS_HLG,   "all"),
    ENUM("sdr",        TS_SDR,   "all"),

    { "displaylight", "displaylight,default 1000nit",
      OFFSET(target_display_light),  AV_OPT_TYPE_INT, { .i64 = 0 },
      -1, 2000, FLAGS, "displaylight" },

    /*{ "pixfmt",   "Output pixel format",
      OFFSET(pixfmt), AV_OPT_TYPE_INT,  { .i64 = AV_PIX_FMT_NONE },
      AV_PIX_FMT_NONE, AV_PIX_FMT_GBRAP12LE, FLAGS, "fmt" },
    ENUM("yuv420p",   AV_PIX_FMT_YUV420P,   "fmt"),
    ENUM("yuv420p10le", AV_PIX_FMT_YUV420P10LE, "fmt"),
    ENUM("yuv420p12", AV_PIX_FMT_YUV420P12, "fmt"),
    ENUM("yuv422p",   AV_PIX_FMT_YUV422P,   "fmt"),
    ENUM("yuv422p10", AV_PIX_FMT_YUV422P10, "fmt"),
    ENUM("yuv422p12", AV_PIX_FMT_YUV422P12, "fmt"),
    ENUM("yuv444p",   AV_PIX_FMT_YUV444P,   "fmt"),
    ENUM("yuv444p10", AV_PIX_FMT_YUV444P10, "fmt"),
    ENUM("yuv444p12", AV_PIX_FMT_YUV444P12, "fmt"),
    ENUM("nv12p10", AV_PIX_FMT_CUDA, "fmt"),*/

    { "deviceid", "device id for gpu,-1 for cpu",
      OFFSET(deviceid),  AV_OPT_TYPE_INT, { .i64 = 0 },
      -1, 64, FLAGS, "deviceid" },

    { "highpointmode", "highpoint_mode :0 HISRGB; 1:HISMAX; 2:MAXSOURCE",
      OFFSET(highpoint_mode),  AV_OPT_TYPE_INT, { .i64 = 0 },
      -1, 64, FLAGS, "highpointmode" },

    { NULL },
};

static const AVClass arcdtransfer_class = {
    .class_name       = "arcdtransfer",
    .item_name        = av_default_item_name,
    .option           = arcdtransfer_options,
    .version          = LIBAVUTIL_VERSION_INT,
    .category         = AV_CLASS_CATEGORY_FILTER,
};

static av_cold int init(AVFilterContext *ctx)//先arcdtransfer_options 获取设置  再init  再query_formats  再conf_input 再filter_frame
{
    av_log(NULL, AV_LOG_DEBUG, "arcdtransfer init!\n");
    ArcdTransferContext *s = ctx->priv;
    s->cuvainit = 0;
    s->pInputPicture = NULL;
    s->pOutPicture = NULL;
    s->pHandle = NULL;

    if(0 == s->target_display_light)
        s->target_display_light = 1000;//need set by usr
    s->bHlgModeOn = 1;
    s->sampleRange = 0;
    s->nRange = 0;
    //s->deviceid = 0;
    s->process_mode = ePostprocessHDR;
   
    s->highpoint_mode = 0;
    s->threshold = 11.0;//整体亮度， 区间为8.0 - 14.0, 默认数值为：11.0;
    s->mpset = 5.0;    //中灰亮度
    s->src_maxluma = 10000;//亮度
    s->src_minluma = 50;
    s->dst_maxluma = 10000;
    s->dst_minluma = 50;
    s->src_color_prim = COLORPRIM_BT2020;  ////0:auto 1:BT709, 9:BT2020
    s->dst_color_prim = COLORPRIM_BT2020; //colour_primaries;
    s->src_transfer = TRANSFER_SDR;
    s->srccolorspace =  IMAGE_CSP_YUV420P10BIT;
    s->dstcolorspace =  IMAGE_CSP_YUV420P10BIT;
    s->pixfmt = ePictureFormatYUV420P10LE;//only support yuv420p10

    s->libHandle = dlopen("libArcHDRCUVA.so", RTLD_LAZY);
    if(s->libHandle == NULL)
    {
        av_log(ctx, AV_LOG_ERROR, "load libArcHDRCUVA.so failed: %s\n", dlerror());
        return AVERROR(EFAULT);
    }
    s->OpenProcessor  = (PCUVAHDR_OPEN_PROCESSER3)dlsym(s->libHandle, "OpenProcessor3");
	s->outputHeader   = (PCUVAHDR_OUTPUT_HEADER)dlsym(s->libHandle, "outputHeader");
	s->createCUVAPicture  = (PCUVAHDR_CREATE_CUVA_PICTURE)dlsym(s->libHandle, "createCUVAPicture");
    s->parserLine2MetaFrame  = (PCUVAHDR_PARSELINE2_META_FRAME)dlsym(s->libHandle, "parserLine2MetaFrame");
    s->PostProcessOneFrame  = (PCUVAHDR_POST_PROCESS_FRAME)dlsym(s->libHandle, "PostProcessOneFrame");
    s->outputFooter  = (PCUDAHDR_OUTPUT_FOOTER)dlsym(s->libHandle, "outputFooter");
    s->CloseProcessor  = (PCUVAHDR_CLOSE_PROCESSER)dlsym(s->libHandle, "CloseProcessor");
    s->destoryCUVAPicture  = (PCUVAHDR_DESTORY_CUVAPicture)dlsym(s->libHandle, "destoryCUVAPicture");

    if (s->OpenProcessor == NULL || s->outputHeader == NULL || 
		s->createCUVAPicture == NULL || s->parserLine2MetaFrame == NULL ||
		s->PostProcessOneFrame == NULL || s->outputFooter == NULL ||
        s->CloseProcessor == NULL || s->destoryCUVAPicture == NULL)
    {
        av_log(ctx, AV_LOG_ERROR, "get libArcHDRCUVA.so api failed\n");
        return AVERROR(EFAULT);
    }
    return 0;
}

static int query_formats(AVFilterContext *ctx)
{
    static const enum AVPixelFormat pixel_fmts[] = {
         
        AV_PIX_FMT_YUV420P,   AV_PIX_FMT_YUV422P,   AV_PIX_FMT_YUV444P,
        /*AV_PIX_FMT_YUV420P10,*/ AV_PIX_FMT_YUV422P10, AV_PIX_FMT_YUV444P10,
        AV_PIX_FMT_YUV420P12, AV_PIX_FMT_YUV422P12, AV_PIX_FMT_YUV444P12,
        AV_PIX_FMT_YUVJ420P,  AV_PIX_FMT_YUVJ422P,  AV_PIX_FMT_YUVJ444P,
        AV_PIX_FMT_P016LE,AV_PIX_FMT_NV12,AV_PIX_FMT_YUV420P10LE,AV_PIX_FMT_CUDA,
        AV_PIX_FMT_NONE
    };

    AVFilterFormats *fmts_list = ff_make_format_list(pixel_fmts);
    if (!fmts_list)
        return AVERROR(ENOMEM);
    return ff_set_common_formats(ctx, fmts_list);
}

static int set_param(ArcdTransferContext *s, AVFrame *in)
{
    if (AVCOL_PRI_BT709 == in->color_primaries)
    {
        s->src_color_prim = COLORPRIM_BT709;//default BT2020
    }
   
    //set s->srccolorspace
    if(in->format == AV_PIX_FMT_YUV420P10LE)//src only support 10bit
    {
        s->srccolorspace = IMAGE_CSP_YUV420P10BIT;
    }
    else if (in->format == AV_PIX_FMT_YUV422P10BE)
    {
        s->srccolorspace = IMAGE_CSP_YUV422P10BIT;
    }
    else if (in->format == AV_PIX_FMT_CUDA)//AV_PIX_FMT_CUDA?
    {
        s->srccolorspace = IMAGE_CSP_NV12P10BIT;
    }

    //set s->src_transfer
    if(in->color_trc == AVCOL_TRC_SMPTE2084)
    {
        s->src_transfer = TRANSFER_HDR10;//16 PQ
    }
    else if (in->color_trc == AVCOL_TRC_ARIB_STD_B67)
    {
        s->src_transfer = TRANSFER_HLG10;//18
    }
    else if (in->color_trc == AVCOL_TRC_BT709)
    {
        s->src_transfer = TRANSFER_SDR;//1
    }

    //根据dst_transfer设置dst_color_prim和process_mode
    if(s->dst_transfer == TS_SDR)
    {
        if(s->src_transfer == TRANSFER_SDR)
        {
            av_log(NULL, AV_LOG_ERROR, "sdr to sdr not supported!\n");
            return -1;
        }
        s->dst_transfer = TRANSFER_SDR;// 1   会转成picture的dst_transfer  0:auto    1:BT709, 14:BT202010bit  16:PQ 18:HLG  ?
        s->dst_color_prim = COLORPRIM_BT709;
        s->process_mode = ePostprocessSDR;
    }else if(s->dst_transfer == TS_HLG)
    {
        s->dst_transfer = TRANSFER_HLG10;//18
    }else if(s->dst_transfer == TS_PQ)   //defalt PQ
    {
        s->dst_transfer = TRANSFER_HDR10;//16
    }
    return 0;
}

static int filter_frame(AVFilterLink *link, AVFrame *in)
{
    AVFilterContext *avctx = link->dst;
    ArcdTransferContext *s = avctx->priv;
    AVFilterLink *outlink = avctx->outputs[0];

    int ret;

    char vivid_one[256] = {0};
    //printf("!!!jl:do_postprocess: get nMod:  %d ;get frame's  side data:%d  frame's w %d h %d format:%d \n", s->nMode, in->nb_side_data, in->width, in->height, in->format);
    AVFrameSideData *side_data = av_frame_get_side_data(in, AV_FRAME_DATA_DYNAMIC_HDR_VIVID);
    if(side_data == NULL){
        av_log(NULL, AV_LOG_ERROR, "no meta data, cannot filter frame using filter: arcdtransfer!\n");
        return -1;
    }
    AVDynamicHDRVivid *vivid_meta_ctx = (AVDynamicHDRVivid *)side_data->data;
    static int frame_num = 0;
    frame_num++;
    ret = vivid_meda_dump(frame_num, vivid_meta_ctx, NULL, vivid_one);
    if(ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "vivid_meda_dump failed, exit!\n");
        return -1;
    }
 
    /*FILE* fp = NULL;
    FILE* fp1 = NULL;
    if (frame_num == 1){
        fp = fopen("wrout.yuv", "w");
        fp1 = fopen("wrin.yuv", "w");
    }
    else{
        fp = fopen("wrout.yuv", "a+");
        fp1 = fopen("wrin.yuv", "a+");
    }*/

     av_log(NULL, AV_LOG_DEBUG,"frame in: linesize[0]=%d  linesize[1]=%d\n", in->linesize[0], in->linesize[1]);

    if(s->cuvainit == 0)
    {
        if(set_param(s, in) < 0)
            return -1;
        av_log(NULL, AV_LOG_DEBUG, "set param: src_color_prim :%d[1:BT709 9:BT2020]  src_transfer=%d [16: pQ  18: HLG  1:SDR] srccolorspace=%d \n", 
        s->src_color_prim, s->src_transfer, s->srccolorspace);
        av_log(NULL, AV_LOG_DEBUG, "OpenProcessor3: process_mode :%d  dst_transfer=%d [16: pQ  18: HLG  1:SDR] displaylight=%d pixfmt:%d[1:yuv42210le 2:yuv42010le 3:nv12] deviceid:%d\n", 
        s->process_mode, s->dst_transfer, s->target_display_light, s->pixfmt, s->deviceid);

        s->pHandle = s->OpenProcessor(s->process_mode, eHDR10, 0, in->width,in->height,s->target_display_light,s->pixfmt,1,0,s->highpoint_mode,s->threshold,s->mpset,s->deviceid);
        if (s->pHandle == NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "OpenProcessor() failed, exit!\n");
            return -1;
        }
        s->outputHeader(s->pHandle); //print infomation 

        s->pInputPicture = (stPicture*)malloc(sizeof(stPicture));
        memset(s->pInputPicture,0,sizeof(stPicture));
        s->pInputPicture  = s->createCUVAPicture(s->pInputPicture, in->width, in->height, s->pixfmt);

        s->pOutPicture = (stPicture*)malloc(sizeof(stPicture));
        memset(s->pOutPicture,0,sizeof(stPicture));
        s->pOutPicture = s->createCUVAPicture(s->pOutPicture, in->width, in->height, s->pixfmt);//&m_OutputPicture;
        
        s->pOutPicture->srccolorspace  = s->dstcolorspace;

        s->cuvainit = 1;
    }

    for (int i = 0; i < 3; i++)
    {
        memcpy(s->pInputPicture->pData[i], in->data[i], s->pInputPicture->nDataLineSize[i]*sizeof(unsigned short));
        //fwrite((char *)s->pInputPicture->pData[i], 1, s->pInputPicture->nDataLineSize[i] * sizeof(unsigned short), fp1);
        //printf("!!!jl: pInputPicture->pData[i] :%p  pInputPicture->nDataLineSize[i]=%d  in->linesize[i]=%d\n", s->pInputPicture->pData[i], s->pInputPicture->nDataLineSize[i], in->linesize[i] );
    }
    //fclose(fp1);
    
    CUVA_metadata  metadata_frame;
    s->parserLine2MetaFrame(s->pHandle, &metadata_frame ,vivid_one);

    
    s->pInputPicture->pbIn = s->pInputPicture->pData[0];
    s->pOutPicture->pbIn = s->pOutPicture->pData[0];
    //视频基本信息
    s->pInputPicture->srcbufmode = 0;//   //0:cpu, 1:cuda
    s->pInputPicture->pCudaContext = NULL;	 //cuda context 
    s->pInputPicture->srctop = 0;              //in unit of pixel
    s->pInputPicture->srcleft = 0;             //in unit of pixel
    s->pInputPicture->srcwidth = in->width;      //in unit of pixel
    s->pInputPicture->srcheight = in->height;    //in unit of pixel
    s->pInputPicture->srcstride = in->width;;     //in unit of pixel
    s->pInputPicture->srcvstride = in->height;   //in unit of pixel
    s->pInputPicture->target_display_light = s->target_display_light;
    
    s->pInputPicture->srccolorspace = s->srccolorspace;
    //视频高级信息 
    s->pInputPicture->src_color_prim = s->src_color_prim;
    s->pInputPicture->src_transfer = s->src_transfer;
    s->pInputPicture->dst_transfer = s->dst_transfer;
        
    s->pInputPicture->src_maxluma	= s->src_maxluma;
    s->pInputPicture->src_minluma  = s->src_minluma;
    s->pInputPicture->dst_color_prim = s->dst_color_prim; //colour_primaries
    s->pInputPicture->dst_maxluma	 = s->dst_maxluma;
    s->pInputPicture->dst_minluma	 = s->dst_minluma;    

   
    s->PostProcessOneFrame(s->pHandle, s->pInputPicture, &metadata_frame, s->pOutPicture);
    av_log(NULL, AV_LOG_DEBUG, "OutPicture: pOutPicture->nWidth=%d\n", s->pOutPicture->nWidth);
    //printf("!!!jl: outlink->w=%d  outlink->h=%d outlink->format=%d\n", outlink->w, outlink->h, outlink->format);
    AVFrame* out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
    if (!out) {
        av_frame_free(&in);
        av_log(NULL, AV_LOG_ERROR, "cannot get out frame! free frame in!\n");
        return AVERROR(ENOMEM);
    }

    for (int i = 0; i < 3; i++)
    {
        av_log(NULL, AV_LOG_DEBUG, "out->linesize[i]:%d \n", out->linesize[i]);
        //out->data[i] = s->pOutPicture->pData[i];
        memcpy(out->data[i], s->pOutPicture->pData[i],  s->pOutPicture->nDataLineSize[i]* sizeof(unsigned short));
        //fwrite((char *)s->pOutPicture->pData[i], 1, s->pOutPicture->nDataLineSize[i] * sizeof(unsigned short), fp);
    }
    //fclose(fp);

    av_frame_copy_props(out, in);
   
    if (out)
    {
        av_frame_free(&in);
        ret = ff_filter_frame(outlink, out);
        if (ret <0 ){
            printf("ff_filter_frame error=%d\n",ret);
        }
    }
    else
        ret = ff_filter_frame(outlink, in);

    return ret;

}


static int config_props(AVFilterLink *outlink)
{
    AVFilterContext *ctx = outlink->dst;
    AVFilterLink *inlink = outlink->src->inputs[0];

    if (inlink->w % 2 || inlink->h % 2) {
        av_log(ctx, AV_LOG_ERROR, "Invalid odd size (%dx%d)\n",
               inlink->w, inlink->h);
        return AVERROR_PATCHWELCOME;
    }

    outlink->w = inlink->w;
    outlink->h = inlink->h;
    outlink->sample_aspect_ratio = inlink->sample_aspect_ratio;
    outlink->time_base = inlink->time_base;

    return 0;
}

static av_cold void uninit(AVFilterContext *ctx)
{ 
    av_log(NULL, AV_LOG_DEBUG, "arcdtransfer  uninit!\n");
    
    ArcdTransferContext *s = ctx->priv;
    if(s->cuvainit == 1)
    {
        s->outputFooter(s->pHandle);
        s->CloseProcessor(s->pHandle);
    }
    
    if(s->pInputPicture)
    {
        s->destoryCUVAPicture(&s->pInputPicture);
    }
    if(s->pOutPicture)
    {
        s->destoryCUVAPicture(&s->pOutPicture);
    }
    //av_frame_free(&s->out);
    av_log(NULL, AV_LOG_DEBUG, "arcdtransfer  uninit end!\n");
}


static const AVFilterPad avfilter_vf_arcdtransfer_inputs[] = {
    {
        .name         = "default",
        .type         = AVMEDIA_TYPE_VIDEO,
        .filter_frame = filter_frame,
        //.config_props = config_input,
    },
};

static const AVFilterPad avfilter_vf_arcdtransfer_outputs[] = {
    {
        .name         = "default",
        .type         = AVMEDIA_TYPE_VIDEO,
        .config_props = config_props,
    },
};


//AVFILTER_DEFINE_CLASS(arcdtransfer);

const AVFilter ff_vf_arcdtransfer = {
    .name            = "arcdtransfer",
    .description     = NULL_IF_CONFIG_SMALL("Apply arcvideo transfer."),
    .init            = init,
    .priv_size       = sizeof(ArcdTransferContext),
    .priv_class      = &arcdtransfer_class,
    .uninit          = uninit,
    FILTER_INPUTS(avfilter_vf_arcdtransfer_inputs),
    FILTER_OUTPUTS(avfilter_vf_arcdtransfer_outputs),
    FILTER_QUERY_FUNC(query_formats),
    //.process_command = process_command,
    .flags           = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC | AVFILTER_FLAG_SLICE_THREADS,
};
