/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Huawei Technologies Duesseldorf GmbH
 * <ORGANIZATION> =   Huawei Technologies Duesseldorf GmbH
 * <YEAR> = 2020
 *
 * Copyright (c) 2020,  Huawei Technologies Duesseldorf GmbH
 * All rights reserved.
 *
 * <OWNER> = Huawei Technologies Co. Ltd.
 * <ORGANIZATION> =   Huawei Technologies Co. Ltd.
 * <YEAR> = 2020
 *
 * Copyright (c) 2020,  Huawei Technologies Co. Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the <ORGANIZATION> nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*!
  *************************************************************************************
  *
  * \brief
  *     Declaration of API for buffer in and out
  *
  * \author
  *     - Hu Chen               <huchen15@huawei.com>
  *     - Yichuan Wang          <wangyichuan7@huawei.com>
  *     - Weiwei Xu             <xuweiwei3@huawei.com>
  *     - QuanHe Yu             <yuquanhe@huawei.com>
  *************************************************************************************
  */

#ifdef __cplusplus
extern "C" {
#endif

# pragma once
//��Ƶ��ʽ��ɫ�ռ䶨��
enum 
{
	ePictureFormatUnknown = 0,
	ePictureFormatYUV422P10LE,
	ePictureFormatYUV420P10LE,
	ePictureFormatNV1210LE,
	ePictureFormatYRGB444
};

enum
{
	eHDR10 = 0,
	eHLG
};
//processMode
enum MODE
{
	ePreprocess = 0,		//generate metadata 
	ePostprocessHDR,		//display adaptation HDR, PQ,HLG... depends dstColorPrim dstTransfer 
	ePostprocessSDR,		// 
	ePreprocessOETF,	    //generate metadata widh  convert OETF
	eProcessOETF,			//just do OETF,no metadata for 3dlut
	ePostProcessCurve		//from metadata to curve 
};


typedef struct CUVA_metadata_t
{
	unsigned int system_start_code;
	unsigned int minimum_maxrgb;
	unsigned int average_maxrgb;
	unsigned int variance_maxrgb;
	unsigned int maximum_maxrgb;
	unsigned int tone_mapping_mode;
	unsigned int tone_mapping_param_num;
	unsigned int targeted_system_display_maximum_luminance[2];
	unsigned int Base_flag[4];
	unsigned int Base_param_m_p[2];
	unsigned int Base_param_m_m[2];
	unsigned int Base_param_m_a[2];
	unsigned int Base_param_m_b[2];
	unsigned int Base_param_m_n[2];
	unsigned int Base_param_K1[2];
	unsigned int Base_param_K2[2];
	unsigned int Base_param_K3[2];
	unsigned int base_param_Delta_mode[2];
	unsigned int base_param_Delta[2];
	unsigned int P3Spline_flag[2];
	unsigned int P3Spline_num[2];
	unsigned int P3Spline_TH_mode[2][4];
	unsigned int P3Spline_TH_MB[2][4];
	unsigned int P3Spline_TH[2][4][3];
	unsigned int P3Spline_Strength[2][4];
	unsigned int color_saturation_mapping_flag;
	unsigned int color_saturation_num;
	unsigned int color_saturation_gain[16];
}CUVA_metadata;

//Ԫ���ݶ�Ӧ������߲���
typedef struct _Cuva_Curve
{
	double m_p;
	double m_m;
	double m_a;
	double m_b;
	double m_n;
	double K1;
	double K2;
	double K3;
	unsigned int base_param_Delta_mode;
	int    curve_mintiao ;
	double TH1, TH2, TH3;
	double md1,mc1,mb1,ma1;
	double md2, mc2, mb2, ma2;
	double DARKcurble_S1;
	double Light_S1;
	double DARKcurble_offset;
	int    curve_mintiao_high_area;
	double TH1_HIGH, TH2_HIGH, TH3_HIGH;
	double md1_high, mc1_high, mb1_high, ma1_high;
	double md2_high, mc2_high, mb2_high, ma2_high;
	int    high_area_flag;
	double curve_adjust;
	double m_p_T;
	double m_a_T;
	double base_param_Delta;
	double P3Spline_Strength[2];
	double P3Spline_TH_MB[2];
	double P3Spline_TH[16][3];
	unsigned int P3Spline_TH_num;
	unsigned int P3Spline_TH_mode[16];
}Cuva_Curve;

//ͼ���ʽ����
typedef struct  _stPicture
{
	/*------------����for ԭʼ�������-----------*/
	//YUV������Buffer����,
	unsigned char *pData[3];
	//YUV�����Ŀ��
	int nDataLineSize[3];
	//��Ƶ�ĸ߶�
	int nWidth;
	//��Ƶ�Ŀ���
	int nHeight;
	//��Ƶ��ʽ
	int nFormat;  //ePictureFormatYUV422P10LE 
    /*------------------------*/

    void* pbIn;   //pbIn !=0 ����pData,������pData
	//��Ƶ������Ϣ
	int srcbufmode;   //0:cpu, 1:cuda
	void* pCudaContext;	//cuda context  
	void* pCudaStream;	//cuda stream 
	//void* pCudaStream;
	int srctop;       //in unit of pixel
	int srcleft;      //in unit of pixel
	int srcwidth;     //in unit of pixel
	int srcheight;    //in unit of pixel
	int srcstride;    //in unit of pixel
	int srcvstride;   //in unit of pixel

	//��Ƶ�߼���Ϣ 
	int srccolorspace; //product video fmt 
    int src_color_prim;
	int src_transfer;
	int src_maxluma;
	int src_minluma;
	int dst_color_prim; //colour_primaries
	int dst_transfer;   //transfer_characteristics
	int dst_maxluma;
	int dst_minluma;
	int target_display_light; //Ŀ����ʾ�豸������� Ĭ��500 0��ʾ�������ļ���

	//Ԫ������Ϣ
	unsigned char *pMetaData;

	//Ԫ������Ϣ��С
	int nMetaDataSize;

	//������ dynamic_metadata()�ṹ
	unsigned char metaDataBin[512];
	int metaBinSize;

	/*
	stPicture(int width, int height, int format);

	~stPicture() {
		//for (int i = 0; i < 3; i++)
		if(pData[0])
			delete[] pData[0];
		if(pMetaData)
			delete[] pMetaData;
	}
	*/
}stPicture;

typedef struct vividProcContext
{
        int    processMode; 		 //0:Ԥ������ȡԪ����ģʽ,1:vivid �������PQģʽ 2���������SDRģʽ; todo .. 
		//int nHDRType;
		int    colorRange;  		 //0:STANDARD ,1: FULL ; SR_RESTRICTED = 2; SR_SDI_SCALED = 3;      
		int    srcWidth;	 
		int    srcHeight;

		float  srcMaxDisplayLight;   //ԭ���������С����
		float  srcMinDisplayLight;
		float  dstMaxDisplayLight;   //Ŀ���������
		float  dstMinDisplayLight;
	    
		int    srcColorRange;  		  //0:STANDARD ,1: FULL ,SR_RESTRICTED = 2; SR_SDI_SCALED = 3;      
		int    dstColorRange;
		int    srcColorPrim;   		  //0:auto 1:BT709, 9:BT2020
        int    dstColorPrim;
		int    srcTransfer;    		  //0:auto 1:BT709, 14:BT202010bit  16:PQ 18:HLG
		int    dstTransfer;
 
		int    method;                //0:auto 1:ģʽ1(2021)(���) 2:ģʽ2(2022 for hlg)��ԭ����ԭ��)
		float  referenceDisplayLight; //Ԫ������������ʱ�ο����Ŀ������

		int    hlgContent;			 //�Ƿ�ԭʼHLG����,ģʽ1��Ч
        //����3������ ģʽ2��Ч
		int    highpointMode;  	  	 //�߹�ģʽ�� 0����ͨ��(rgbȡƽ��,Ĭ��); 1:��ͨ��(rgb ȡ������ͨ��ƽ��ֵ); 2:ԭʼͨ����rgbMax���ֵ); 
		double threshold;         	 //�������ȣ� ����Ϊ8.0 - 14.0, Ĭ����ֵΪ��11.0;
		double mpset;			  	 //�л�����:  ����Ϊ2.0 - 8.0, Ĭ����ֵΪ��5.0;
		//Ԥ�����²�����
		int    downsampleMode;  	 //�²���ģʽ��0: ԭʼ�ֱ���; 1:�����л��²���,2:��ȡ�����²���
		int    downsampleType;   	 //�²����㷨: 0: ʹ��ƽ��ֵ; 1:ʹ�õ�0�� 2��ʹ���������ֵ 3��ʹ��������Сֵ;
		
		int    forceInternalCtx;	 //ǿ���ڲ�ʹ���µ�context��custream;
		int    forceInternalStream;	 //ǿ���ڲ�ʹ���µ�custream;

		int   deviceType;  		 	 //Ŀ������豸���ͣ�0:cpu;1:cuda;2?
		int   deviceId;			 	 //Ŀ������豸Id	-1��cpu
		void* deviceHandle;			 //Ŀ���豸�����û�����ڲ�����
		void* deviceContext;		 //Ŀ���豸context
		void* deviceStream;			 //Ŀ���豸stream ,Ĭ����frame֡�ڴ���
		char* lutUrl;				 //lut��URL 	
		int   lut_mode;				 //lutģʽ 0:typeA 1:typeB (����Ĭ��,���������0-1023������typeA��64-940����(�ݲ�֧�֣���Щϵͳ��֧��)))
		int   lut_range;             //0:narrow (default,to limit:1023*fR) 1:full (to limit: fR*(876/1023)+(64/1023)) 
		// vivid meta control params (13*2-1)
		int    iAutoMode;      //���������Զ�����ģʽ,��̬�������²���,0 ԭʼ 1 �ֶ� 2 �Զ���������ģʽ
		double dark_floor;     //th1 λ�� 
		double dark_hillside;  //th2 λ��
		double light_hillside; //th2_high λ��

		/*---����ƫ�� + �л����� =>����(th3,th3_y)ê��λ��(x��,y��)---*/
		//@name:��(��)������(б��); @action: ����һ����������б�ʸ��� dark_s1
		double dark_light;  //[-1,1]; ���鷶Χ[0-1]; -1=>0; 1=>1 0=>MB_ref(��ǰֵ)
		//@name:����ƫ��; @action: �������������߲������� m_p,m_a,m_b,m_m,m_n,K1,K2,K3 ...
		double dark_offset; //[-1,1]; ����th3, 0=th3_ref,-1=>th2_ref,1=>0.45  ���鷶Χ:[th2_ref,0.45]
		//@name:����ϸ��; @action: ����3����������ǿ�ȸ��� P3Spline_Strength_ref[0]
		double dark_detail; //[-1,1]; ���鷶Χ[-0.5,0.5], -1=>-0.5;1=>0.5; 0=>P3Spline_Strength_ref[0] 
		//@name:�л�����(Ŀ��ӳ������); @action:����th3��ӦYֵ,���������߲�������
		double mid_light;//[-1,1]; ���鷶Χ[th2_y_ref,th1_high_y_ref]; 
	
		/*---����ƫ�� + �������� =>����(th1_high,th1_high_y)ê��λ��(x��,y��)---*/

		//@name:����ƫ��; @action:����th1_high,���������߲������¼���
		double high_offset;//[-1,1];���鷶Χ[0.48,,th2_high_ref]; -1=>0.48; 1=>th2_high 
		//@name:��������(Ŀ��ӳ������); @action:����th1_high_y ֵ,�������������߲������¼���
		double high_light;//[-1,1];��Ӧ (th3_y_ref-th2_high_ref)
		//@name:����ϸ�� @action: ����3����������ǿ�ȸ��� P3Spline_Strength_ref[1]
		double high_detail;//[-1,1]; ���鷶Χ[-0.5,0.5], -1=>-0.5;1=>0.5; 0=>P3Spline_Strength_ref[1] 
		
		//@name:�߹�(������������offset)  @action:����th3_high ;ȷ��th3_high_y ���ڲο���ʾ���������
		double high_cut;//[-1,1],���鷶Χ[TH2_HIGH,max_source],max_source ��ǰ֡rgb_max ���PQֵ

		//���Ͷ�
		//@name:���履�Ͷ�;@action: ���� color_saturation_gain[0]
		double whole_sat; //[-1,1]; 0=>��ǰֵ  -1 =>0; 1=>255
		//@name:�߹ⱥ�Ͷ�;@action: ���� color_saturation_gain[1]
		double high_sat; //[-1,1]; 0=>��ǰֵ  -1 =>0;  1=>255

		int   reserved[64-13*2-1-1-1];
}vividProcContext;

typedef enum VividErrorCode
{
	VIVID_SUCCESS = 0,
	VIVID_CUDA_INIT_FAILED = 1,    //err_msg �м������Ӿ������������Ϣ
    VIVID_CUDA_RUN_FAILED  = 1,

}VividErrorCode;


int  CheckEnvironment(int gpu_idx);

//nMode�������ô�������, nMode���ٰ�������ģʽ: Ԥ����ģʽ ������ʾģʽ
//nHDRType����������HDR10 HLG��Ϣ
//���ص�ָ�����pHandle
void* OpenProcessor(int nMode, int nHDRType);

//���ص�ָ�����pHandle
//nRange:	SR_STANDARD = 0, SR_FULL = 1, SR_RESTRICTED = 2, SR_SDI_SCALED = 3,SR_SDI = 4,   
//pixfmt: ePictureFormatYUV422P10LE ,ePictureFormatYUV420P10LE
//device <0 , ����ԭʼCPU���� ����device ����GPU id
//void* OpenProcessor2(int nMode, int nHDRType,int nRange,int width,int height,int pixfmt,int target_display_light,int device);
//bHlgProcOn = 1 default 
void* OpenProcessor2(int nMode, int nHDRType,int nRange,int width,int height,int pixfmt,int target_display_light,int bHlgModeOn,int sampleRange,int device);

// enum BaseHighPoint
// {
// 	HISRGB = 0,
// 	HISMAX = 1,
// 	MAXSOURCE
// };
//add 3 more parameters :
//highpoint_mode :0 HISRGB; 1:HISMAX; 2:MAXSOURCE  //is a kind of mode,�߹���� �߹�ģʽ
//threshold  // default 11.0  �������Ȳ��� [8.0-14.0]
//mpset      // default 5.0   �л����Ȳ���  [2.0-8.0]  are "double" parameters that we want to adjust, His3  the whole set is[His, His3, Max], it also needs to be chosen flexibly. 
void* OpenProcessor3(int nMode, int nHDRType,int nRange,int width,int height,int pixfmt,int target_display_light,int bHlgModeOn,int sampleRange,int highpoint_mode, double threshold,double mpset ,int device);
//��������:


//final open�ӿ�,���в�����context����չ;
void* OpenProcessor4(vividProcContext* context,int *err_code,char* err_msg,int* err_len);

//Ԥ��������ģʽ
//1. ����һ֡����, pInputPicture�е�pMetaDataΪ��, ��ǰ��׼���õ����ݾ������ppOutputPicture, ppOutputPicture�е�pMetaDataΪԤ�������Ԫ������Ϣ, û��׼��������*ppOutputPicture��ΪNULL, Ȼ��һֱ��������
//2. ��������ʱ��ѭ�����ò�����pInputPicture=NULL��ֱ���ӿڷ����Ѿ�����
//3. �ӿڷ���ֵ���ٰ���3��״̬�� 1.����  2.�ɹ�  3.�Ѿ�����
//��ʾ����ģʽ
//1. ����һ֡����, pInputPicture�е�pMetaDataΪԪ������Ϣ, ��ǰ��׼���õ����ݾ������ppOutputPicture, û��׼����*ppOutputPicture��ΪNULL, Ȼ��һֱ��������
//2. ��������ʱ��ѭ�����ò�����pInputPicture=NULL��ֱ���ӿڷ����Ѿ�����
//3. �ӿڷ���ֵ���ٰ���3��״̬�� 1.����  2.�ɹ�  3.�Ѿ�����
int ProcessPicture(void* pHandle, int frm_id, stPicture const *pInputPicture, stPicture **ppOutputPicture);

//Ԥ����һ֡����,��metadata�����meta dada ����ڽṹ�壺unsigned char *pMetaData; �����ƣ�metaDataBin��
int PreProcessOneFrame(void* pHandle, stPicture *pInputPicture);


//����һ֡���ݣ���metadata)���������֡
int  PostProcessOneFrame(void* pHandle, stPicture const *pInputPicture, CUVA_metadata*pMeta, stPicture *pOutputPicture);

//��Ԫ��������ӳ�����߲���
//postMode = 1(HDR),2(SDR)
int  PostProcessMeta2Curve(void* pHandle, CUVA_metadata* pMetaIn,Cuva_Curve *pCurveOut,float target_max_display,float target_min_display,float master_max_display,int postMode);

// ��һ֡����ϵ������д��һ���ļ�
int  packetCurve2File(void* pHandle,int framenum ,Cuva_Curve *pCurve, char* metafile);
// ��һ���ļ�������һ֡���߲�����Ϣ
int  parserLine2Curve(void* pHandle,Cuva_Curve *pCurve,const char* line);

// �������߲�������ӳ���
//PQ[0-1]=>PQ[0-1] 
int  CalCurveLutNoLinear(void* pHandle,Cuva_Curve *pCurve,double* pResult,int size);

//����ĳ���������䣨nit<=10000�� ӳ�䵽��Ŀ����������
int  CalCurveLutLinear(void* pHandle,Cuva_Curve *pCurve,double* pResult,int size,int start_luma,int end_luma);


int  packetLut2File(void* pHandle,int framenum ,double* pResult,int size, char* lutfile);


//Vivid����������ת����HLG<->PQ ;todo ..
int  PreprocessOneFrameOETF(void* pHandle, stPicture const *pInputPicture,  stPicture *pOutputPicture,CUVA_metadata *poutMeta);

//������ת����HLG<->PQ,HDR<->SDR;todo ..
int  ProcessOneFrameOETF(void* pHandle, stPicture const *pInputPicture,  stPicture *pOutputPicture);

int  ProcessOneFrameOETFEX(void* pHandle, stPicture const *pInputPicture,  stPicture *pOutputPicture, int device);

//��һ��metadata�ļ��������ɽṹ��
int  parserLine2MetaFrame(void* pHandle,CUVA_metadata*pMeta,const char* line);

//��һ��������dynamic metadata ���ݿ飬������һ֡metadata
int  parserBin2MetaFrame(void* pHandle,CUVA_metadata*pMeta,const char* pData,int nLen);


//��ӡһ��metadata ��ӡ���ļ�
int  packetMetaFrame2File(void* pHandle,int framenum ,CUVA_metadata*pMeta, char* metafile);

//���һ֡metadata ��һ��������dynamic metadata �ṹ������ len
int  packetMetaFrame2Bin(void* pHandle,CUVA_metadata*pMeta,char* pData);


//��һ��������AVS2/AVS3 dynamic metadata ���ݿ飬������һ֡metadata 
int  parserBin2MetaFrameAVS(void* pHandle,CUVA_metadata*pMeta,const char* pData,int nLen);

//���һ֡metadata ��һ��������dynamic metadata �ṹ(AVS2/AVS3)������ len
int  packetMetaFrame2BinAVS(void* pHandle,CUVA_metadata*pMeta,char* pData);


//avs2/avs3 add marker_bit ,return dst len
int MetabinAddMarkerBit(void* pHandle, unsigned char* pDst,int dstLen,unsigned char* pSrc,int srcLen);
//avs2/avs3 remove marker_bit, return dst len
int MetabinRemoveMarkerBit(void* pHandle, unsigned char* pDst,int dstLen,unsigned char* pSrc,int srcLen);

//h264/h265 add prevent byte ,return dst len
int MetabinAddPreventByte(void* pHandle, unsigned char* pDst,int dstLen,unsigned char* pSrc,int srcLen);
//h264/h265 remove prevent byte ,return dst len
int MetabinRemovePreventByte(void* pHandle, unsigned char* pDst,int dstLen,unsigned char* pSrc,int srcLen);

int CloseProcessor(void* pHandle);

//print headers 
int outputHeader(void* pHandle);

//print finsined 
int outputFooter(void* pHandle);

//print count ++ 
int paramCountUp(void* pHandle);

int paramCountGet(void* pHandle);

stPicture* createCUVAPicture(stPicture* Picture, int width, int height, int format);
int  destoryCUVAPicture(stPicture** stPic);

typedef void* (*PCUVAHDR_OPEN_PROCESSER3)(int nMode, int nHDRType,int nRange,int width,int height,int pixfmt,int target_display_light,int bHlgModeOn,int sampleRange,int highpoint_mode, double threshold,double mpset ,int device);
typedef int   (*PCUVAHDR_OUTPUT_HEADER)(void* pHandle);
typedef stPicture* (*PCUVAHDR_CREATE_CUVA_PICTURE)(stPicture* Picture, int width, int height, int format);
typedef int   (*PCUVAHDR_PARSELINE2_META_FRAME)(void* pHandle,CUVA_metadata*pMeta,const char* line);
typedef int  (*PCUVAHDR_POST_PROCESS_FRAME)(void* pHandle, stPicture const *pInputPicture, CUVA_metadata* pMeta, stPicture *pOutputPicture);
typedef int  (*PCUDAHDR_OUTPUT_FOOTER)(void* pHandle);
typedef int  (*PCUVAHDR_CLOSE_PROCESSER)(void* pHandle);
typedef int  (*PCUVAHDR_DESTORY_CUVAPicture)(stPicture** stPic);


#ifdef __cplusplus
}
#endif