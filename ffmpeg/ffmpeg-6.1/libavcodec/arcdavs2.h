#ifndef _ASAVS2DEC_H_
#define _ASAVS2DEC_H_

#include <stdint.h>
#include <stdbool.h>

#define CAVS_SEQ_HEADER			            1
#define CAVS_FRAME_OUT			            2
#define CAVS_ERROR				            8
								            
#define CAVS_CS_YUV420			            0
#define CAVS_CS_YUYV			            1

#define CAVS_SLICE_MIN_START_CODE           0x00000100
#define CAVS_SLICE_MAX_START_CODE           0x000001AF
#define CAVS_VIDEO_SEQUENCE_START_CODE      0x000001B0
#define CAVS_VIDEO_SEQUENCE_END_CODE        0x000001B1
#define CAVS_USER_DATA_CODE                 0x000001B2
#define CAVS_INTRA_PICUTRE_START_CODE       0x000001B3
#define CAVS_EXTENSION_START_CODE           0x000001B5
#define CAVS_INTER_PICUTRE_START_CODE       0x000001B6
#define CAVS_VIDEO_EDIT_CODE                0x000001B7
#define CAVS_VIDEO_TIME_CODE                0x000001E0
#define NAL_BUFFER_SIZE                     3840 * 2160

#define SRCHDINFO_RESERVED_NUM	128 - 4 - sizeof(uint8_t*) / sizeof(int)
typedef struct _HDRINFO
{
	//VUI
	uint8_t  colour_description_present_flag;
	uint8_t  colour_primaries;
	uint8_t  transfer_characteristic;
	uint8_t  matrix_coeffs;

	//mastering_display_colour_volume
	uint8_t  mastering_display_flag;
	uint16_t display_primaries_x[3];
	uint16_t display_primaries_y[3];

	uint16_t white_point_x;
	uint16_t white_point_y;
	uint32_t max_display_mastering_luminance;
	uint32_t min_display_mastering_luminance;

	//content_light_level
	uint8_t  content_light_level_flag;
	uint16_t max_content_light_level;
	uint16_t max_pic_average_light_level;

	int32_t	 color_range;
	int32_t  src_force;
	int32_t  metadata_type;
	int32_t  metadata_size;
	int8_t  *metadata;
	int32_t  reserved[SRCHDINFO_RESERVED_NUM];
}HDRINFO;

typedef struct tagasavs2_seq_info
{
	long     lWidth;
	long     lHeight;
	int32_t  i_frame_rate_den;
	int32_t  i_frame_rate_num;
	int32_t  b_progressive_sequence;
	int32_t  profile;
	int32_t  level;
	int32_t  aspect_ratio;
	int32_t  low_delay;
	int32_t  frame_rate_code;
}asavs2_seq_info;

typedef struct tagasavs2_param
{
	uint32_t i_color_space;
	uint8_t* p_out_yuv[3];
	uint32_t seq_header_flag;
	asavs2_seq_info seq_info;
	int32_t  b_interlaced;
	int32_t  output_type;
	uint64_t rtStart;
	int32_t  i_thread_num;
	int32_t  i_decode_mode;
	uint32_t u_out_stride[3];
	HDRINFO  HDR_info;
	uint8_t  field_output_mode;
	uint8_t  decoder_index;
	uint8_t  b_error_notify;
	void*    h_output_image;
	uint8_t  reserved[21];
}asavs2_param;

typedef void(*asavs2_callback)(void* pData);
typedef int (*PAVS2_DECODER_CREATE)(void **pDecoder, asavs2_param *param, asavs2_callback pCallback, void* pCallBackData);
typedef int (*PAVS2_DECODER_INIT_STREAM)(void *pDecoder, uint8_t *rawStream, uint32_t len);
typedef int (*PAVS2_DECODER_GET_NAL)(void *pDecoder, uint8_t *buf, int bufSize, int *length);
typedef int (*PAVS2_DECODER_PROCESS)(void *pDecoder, uint8_t *pIn, int len);
typedef void(*PAVS2_DECODER_DESTROY)(void *pDecoder);
typedef int (*PAVS2_DECODER_GET_SEQ)(void *pDecoder, asavs2_seq_info *pSeq);
typedef int (*PAVS2_DECODE_FRAME)(void *pDecoder, int startcode, asavs2_param *param, uint8_t* buf, int length);
typedef int (*PAVS2_DECODER_REALLOC)(void *pDecoder);
typedef int (*PAVS2_DECODER_PROBE_SEQ)(void *pDecoder, uint8_t *pIn, int length);
typedef int (*PAVS2_DECODER_PIC_HEADER)(void* pDecoder, uint8_t *pBuf, int len, asavs2_param* param, uint32_t startcode);
typedef int (*PAVS2_DECODER_FLUSH)(void *pDecoder, bool bFlush);
typedef int (*PAVS2_SEQUENCE_CHANGED)(void *pDecoder);
typedef void(*PAVS2_UNREF_FRAME)(void *pDecoder, void *image);
typedef int (*PAVS2_OUTPUT_FRAME)(void *pDecoder, asavs2_param *param, bool bFlush);
typedef void(*PAVS2_WAIT_FINISH)(void *pDecoder);

#endif

