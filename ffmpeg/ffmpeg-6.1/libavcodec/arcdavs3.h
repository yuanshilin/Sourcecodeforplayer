
#ifndef DAVS3_DAVS3_DEC_API_H
#define DAVS3_DAVS3_DEC_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define DAVS3_API __declspec(dllexport)
#else
#define DAVS3_API
#endif

typedef struct davs3_picture_t        davs3_picture_t;
/* ---------------------------------------------------------------------------
 * information of return value for decode/flush()
 */
enum davs3_ret_e 
{
    DAVS3_ERROR      = -1, /* Decoding error occurs */
    DAVS3_DEFAULT    = 0,  /* Decoding but no output */
    DAVS3_GOT_FRAME  = 1,  /* Decoding get frame */
    DAVS3_GOT_HEADER = 2,  /* Decoding get sequence header */
    DAVS3_GOT_BOTH   = 3,  /* Decoding get sequence header and frame together */
    DAVS3_END        = 4,  /* Decoding ended: no more bit-stream to decode and no more frames to output */
};

/* ---------------------------------------------------------------------------
 * parameters for create an AVS3 decoder
 */
typedef struct davs3_param_t 
{
    int threads;     /* decoding threads: 0 for auto */
    int info_level;  /* @davs3_log_level_e only output information which is no less then this level. */
                     /* 0: All; 1: no debug info; 2: only warning and errors; 3: only errors */
    void *opaque;    /* frames. */

    int md5_check;
	int thread_type; /* 0: frame; 1: slice */
	int decode_mode; /* 0: normal; 1: all intra sync mode */
	int low_delay;    /* 0: normal; 1: init dpm before decode for low delay */
	int quality_optimize; /* 0: normal; 1: fake ref */
	int reserved[14];
} davs3_param_t;

/* ---------------------------------------------------------------------------
 * packet of bitstream
 */
typedef struct davs3_packet_t 
{
    const uint8_t *data; /* bitstream */
    int            len;  /* bytes of the bitstream */
    int64_t        pts;  /* presentation time stamp */
    int64_t        dts;  /* decoding time stamp */
} davs3_packet_t;

/**
 * ===========================================================================
 * interface struct type defines
 * ===========================================================================
 */

typedef struct davs3_seq_display_extension_t 
{
    uint8_t  is_valid;                          /* whether this data field is valid */
    uint8_t  video_format;                     /* 0~7;  0-color component, 1-PAL, 2-NTSC, 3-SECAM, 4-MAC, 5-undefined by AVS3, 6/7 reserved */
    int8_t   sample_range;                     /* 0~1, indicate the range of luma/chroma component samples */
    int8_t   color_description;                /* valid color description */
    uint8_t  color_primaries;                  /* 8bit, 0~255 */
    uint8_t  transfer_characteristics;         /* 8bit, 0~255 */
    uint8_t  matrix_coefficients;              /* 8bit, 0~255 */
    uint16_t display_horizontal_size;          /* 14bit */
    uint16_t display_vertical_size;            /* 14bit */
    int8_t   td_mode_flag;                     /* whether it is 3D mode or not, 0-Single view Video, 1-including multiple views or depth */
    uint8_t  view_packing_mode;                /* 8bit, 3D packing mode */
    int8_t   view_reverse_flag;                /* video reverse flag */
} davs3_seq_display_extension_t;

typedef struct davs3_mastering_display_extension_t 
{
    uint8_t  is_valid;                        /* whether this data field is valid */
    uint16_t display_primaries_x[3];          /* 16bit display_primaries_x[c] */
    uint16_t display_primaries_y[3];          /* 16bit display_primaries_y[c] */
    uint16_t white_point_x;                   /* 16bit white_point_x */
    uint16_t white_point_y;                   /* 16bit white_point_y */
    uint16_t max_display_mastering_luminance; /* 16bit */
    uint16_t min_display_mastering_luminance; /* 16bit */
    uint16_t max_content_light_level;         /* 16bit */
    uint16_t max_picture_average_light_level; /* 16bit */
} davs3_mastering_display_extension_t;

/* ---------------------------------------------------------------------------
 * information of sequence header
 */
typedef struct davs3_seq_info_t 
{
    uint32_t profile_id;         /* @davs3_profile_id_e profile ID */
    uint32_t level_id;           /* level   ID */
    uint32_t progressive;        /* progressive sequence. 0: interlace; 1: progressive */
    uint32_t width;              /* image width */
    uint32_t height;             /* image height */
    uint32_t chroma_format;      /* chroma format. 1: YUV420; 2: YUV422) */
    uint32_t aspect_ratio;       /* 2: 4:3; 3: 16:9 */
    uint32_t low_delay;          /* low delay */
    uint32_t bitrate;            /* bitrate (bps) */
    uint32_t internal_bit_depth; /* internal sample bit depth */
    uint32_t output_bit_depth;   /* output sample bit depth */
    uint32_t bytes_per_sample;   /* bytes per sample */
    float    frame_rate;         /* frame rate */
    uint32_t frame_rate_id;      /* frame rate code */
                                 /* 1: 24000/1001; 2: 24; 3: 25; 4: 30000/1001 */
                                 /* 5: 30; 6: 50; 7: 60000/1001; 8: 60 */
    davs3_seq_display_extension_t display;
    davs3_mastering_display_extension_t mastering_display;
} davs3_seq_info_t;

/* ---------------------------------------------------------------------------
 * decoded picture
 */
typedef struct davs3_picture_t
{
    void    *magic;               /* must be the 1st member variable (do not change it) */
    int     num_planes;           /* number of plane */
    int     width[3];             /* width (in unit of pixel) */
    int     height[3];            /* height (in unit of pixel) */
    int     stride[3];            /* buffer stride (in unit of byte) */
    void    *planes[3];           /* address of each plane */
    int     bytes_per_sample;     /* number of bytes per sample */
    int     pic_order_count;      /* picture number */
    int     type;                 /* picture type of the corresponding frame */
    int     qp;                   /* QP of the corresponding picture */
    int64_t pts;                  /* presentation time stamp */
    int64_t dts;                  /* decoding time stamp */
    int     time_decode_ms;       /* decoding time in ms */
    
	/* field infomation */
    int8_t  progressive_frame;
    int8_t  picture_structure;
    int8_t  b_top_field_first;
    int8_t  b_repeat_first_field;
    int8_t  b_top_field;

    /* life cycle management */
    int     refcnt;
    int     (*addref)(davs3_picture_t *pic);
    int     (*getref)(davs3_picture_t *pic);
    int     (*release)(davs3_picture_t *pic);

    int     buf_size;
    uint8_t locked;

	int metadata_type;
	int metadata_size;
	char metadata[1024];
	int is_private_422;
	int memory_size;
	int is_error;
	int reserved[61];
} davs3_picture_t;

/**
 * ===========================================================================
 * interface function declares (DAVS3 library APIs for AVS3 video decoder)
 * ===========================================================================
 */

/**
 * ---------------------------------------------------------------------------
 * Function   : open an AVS3 decoder
 * Parameters :
 *   [in/out] : param - pointer to struct davs3_param_t
 * Return     : handle of the decoder, zero for failure
 * ---------------------------------------------------------------------------
 */
DAVS3_API void *davs3_decoder_open(davs3_param_t *param);

/**
 * ---------------------------------------------------------------------------
 * Function   : send packet data into decoder
 * Parameters :
 *       [in] : decoder   - pointer to the AVS3 decoder handler
 *       [in] : packet    - pointer to struct davs3_packet_t, should always starts with a start code.
 * Return     : see definition of davs3_ret_e
 * ---------------------------------------------------------------------------
 */
DAVS3_API int davs3_decoder_send_packet(void *decoder, davs3_packet_t *packet);

/**
 * ---------------------------------------------------------------------------
 * Function   : get decoded frame
 * Parameters :
 *       [in] : decoder   - pointer to the AVS3 decoder handler
 *      [out] : headerset - pointer to output common frame information (would always appear before frame output)
 *      [out] : out_frame - pointer to output frame information (need to be dumped and call unref outside)
 * Return : see definition of davs3_ret_e
 * ---------------------------------------------------------------------------
 */
DAVS3_API int davs3_decoder_recv_frame(void *decoder, davs3_seq_info_t *headerset, davs3_picture_t *outpic);

/**
 * ---------------------------------------------------------------------------
 * Function   : flush the decoder
 * Parameters :
 *       [in] : decoder   - decoder handle
 *      [out] : headerset - pointer to output common frame information (would always appear before frame output)
 *      [out] : out_frame - pointer to output frame information (need to be dumped and call unref outside)
 * Return : see definition of davs3_ret_e
 * ---------------------------------------------------------------------------
 */
DAVS3_API int davs3_decoder_flush(void *decoder, davs3_seq_info_t *headerset, davs3_picture_t *outpic);

/**
 * ---------------------------------------------------------------------------
 * Function   : release one output frame
 * Parameters :
 *       [in] : decoder   - decoder handle
 *            : out_frame - frame to recycle
 * Return     : none
 * ---------------------------------------------------------------------------
 */
DAVS3_API void davs3_decoder_recycle_frame(void *decoder, davs3_picture_t *outpic);

/**
 * ---------------------------------------------------------------------------
 * Function   : reset the AVS3 decoder
 * Parameters :
 *       [in] : decoder - decoder handle
 * Return     : none
 * ---------------------------------------------------------------------------
 */
DAVS3_API void davs3_decoder_reset(void *decoder);

/**
 * ---------------------------------------------------------------------------
 * Function   : close the AVS3 decoder
 * Parameters :
 *       [in] : decoder - decoder handle
 * Return     : none
 * ---------------------------------------------------------------------------
 */
DAVS3_API void davs3_decoder_close(void *decoder);

typedef void* (*PAVS3_DECODER_OPEN)(davs3_param_t *param);
typedef int   (*PAVS3_DECODER_SEND_PACKET)(void *decoder, davs3_packet_t *packet);
typedef int   (*PAVS3_DECODER_RECV_FRAME)(void *decoder, davs3_seq_info_t *headerset, davs3_picture_t *outpic);
typedef int   (*PAVS3_DECODER_FLUSH)(void *decoder, davs3_seq_info_t *headerset, davs3_picture_t *outpic);
typedef void  (*PAVS3_DECODER_RECYCLE_FRAME)(void *decoder, davs3_picture_t *outpic);
typedef void  (*PAVS3_DECODER_RESET)(void *decoder);
typedef void  (*PAVS3_DECODER_CLOSE)(void *decoder);

#ifdef __cplusplus
}
#endif

#endif  /// DAVS3_DAVS3_DEC_API_H
