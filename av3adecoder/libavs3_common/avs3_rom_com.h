#ifndef AVS3_ROM_COM_H
#define AVS3_ROM_COM_H

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_stat_com.h"


/* bitrate table for each mode */
extern const long bitrateTableMono[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableStereo[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableMC5P1[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableMC7P1[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableMC4P0[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableMC5P1P2[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableMC5P1P4[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableMC7P1P2[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableMC7P1P4[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableFoa[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableHoa2[AVS3_SIZE_BITRATE_TABLE];
extern const long bitrateTableHoa3[AVS3_SIZE_BITRATE_TABLE];

/* Codec bitrate config struct */
extern const CodecBitrateConfig codecBitrateConfigTable[CHANNEL_CONFIG_UNKNOWN];

/* MC channel config table */
/* Cmd line, ChannelNumConfig and numChannels mapping */
extern const McChanelConfig mcChannelConfigTable[AVS3_SIZE_MC_CONFIG_TABLE];

/* Sampling rate table */
extern const long avs3SamplingRateTable[AVS3_SIZE_FS_TABLE];

extern const float hpFilterCoff[HP_ORDER + 1];             // high pass filter for trans detect

/* FFT */
extern const float fft_cos_table32[FFT_TABLE_SIZE32];
extern const float fft_cos_table128[FFT_TABLE_SIZE128];
extern const float fft_cos_table256[FFT_TABLE_SIZE256];

extern const float fft_sin_table32[FFT_TABLE_SIZE32];
extern const float fft_sin_table128[FFT_TABLE_SIZE128];
extern const float fft_sin_table256[FFT_TABLE_SIZE256];

extern const unsigned short fft_reordertbl64[FFT_TABLE_SIZE64];
extern const unsigned short fft_reordertbl256[FFT_TABLE_SIZE256];
extern const unsigned short fft_reordertbl512[FFT_TABLE_SIZE512];

/* HOA */
extern const float avs3_hoa_first_step_sample_point_basis_matrix[L_HOA_BASIS_ROWS][L_FIRST_ORDER_HOA_BASIS];
extern const short avs3_hoa_sample_point_basis_matrix_second_step[L_FIRST_ORDER_HOA_BASIS][L_SECOND_ORDER_MP_BASIS];
extern const short avs3_hoa_fixed_angle_basis_matrix[L_HOA_BASIS_COLS][2];
extern const float avs3_hoa_sin_table[L_SIN_TABLE_256 + 1];
extern const short avs3_mp_vl_sfb[HOA_MAX_SFB];

extern const short hoa_sfb_table_low_bitrate[N_SFB_HOA_LBR];

extern const HOA_GROUP_CONFIG avs3_hoa2_group_config[HOA_SIZE_BITRATE_TABLE];
extern const HOA_GROUP_CONFIG avs3_hoa3_group_config[HOA_SIZE_BITRATE_TABLE];

/* FD spectrum shaping tables */
extern const float grid100[];                          /* Table of 100 grid points for evaluating Chebyshev polynomials */

extern const short sfb_table_fb_long[N_SFB_FB_LONG + 1];
extern const short sfb_len_fb_long[N_SFB_FB_LONG];

extern const float mean_lsf[LPC_ORDER];
extern const float mean_lsp[LPC_ORDER];

// high bitrate LSF CB
extern const float lsf_stage1_CB1_hbr[LSF_STAGE1_CB1_SIZE_HBR * LSF_STAGE1_LEN1_HBR];
extern const float lsf_stage1_CB2_hbr[LSF_STAGE1_CB2_SIZE_HBR * LSF_STAGE1_LEN2_HBR];
extern const float lsf_stage2_CB1_hbr[LSF_STAGE2_CB1_SIZE_HBR * LSF_STAGE2_LEN1_HBR];
extern const float lsf_stage2_CB2_hbr[LSF_STAGE2_CB2_SIZE_HBR * LSF_STAGE2_LEN2_HBR];
extern const float lsf_stage2_CB3_hbr[LSF_STAGE2_CB3_SIZE_HBR * LSF_STAGE2_LEN3_HBR];
extern const float lsf_stage2_CB4_hbr[LSF_STAGE2_CB4_SIZE_HBR * LSF_STAGE2_LEN4_HBR];
extern const float lsf_stage2_CB5_hbr[LSF_STAGE2_CB5_SIZE_HBR * LSF_STAGE2_LEN5_HBR];

// low bitrate LSF CB
extern const float lsf_stage1_CB1_lbr[LSF_STAGE1_CB1_SIZE_LBR * LSF_STAGE1_LEN1_LBR];
extern const float lsf_stage1_CB2_lbr[LSF_STAGE1_CB2_SIZE_LBR * LSF_STAGE1_LEN2_LBR];
extern const float lsf_stage2_CB1_lbr[LSF_STAGE2_CB1_SIZE_LBR * LSF_STAGE2_LEN1_LBR];
extern const float lsf_stage2_CB2_lbr[LSF_STAGE2_CB2_SIZE_LBR * LSF_STAGE2_LEN2_LBR];
extern const float lsf_stage2_CB3_lbr[LSF_STAGE2_CB3_SIZE_LBR * LSF_STAGE2_LEN3_LBR];

/* TNS */
extern const float tnsCoeff4[N_TNS_COEFF_CODES];
extern const TnsHuffman *tnsCodingTable[TNS_MAX_FILTER_ORDER];
extern short FilterBorders[TNS_MAX_FILTER_NUM][2];

/* BWE */
extern const int16_t bweSfbTable[BWE_BITRATE_FB_UNKNOWN][MAX_NUM_SFB_BWE + 2];
extern const int16_t bweTargetTileTable[BWE_BITRATE_FB_UNKNOWN][MAX_NUM_TILE + 2];
extern const int16_t bweSrcTileTable[BWE_BITRATE_FB_UNKNOWN][MAX_NUM_TILE];
extern const int16_t bweSfbTileWrapTable[BWE_BITRATE_FB_UNKNOWN][MAX_NUM_TILE + 1];

/* CRC */
extern const int16_t crc16Table[];

/* MCR */
extern const short mcr_sfb_table_fb[MCR_NUM_SFB_FB + 1];
extern const short mcr_sfb_len_fb[MCR_NUM_SFB_FB];
extern const float mcr_codebook_9bit[MCR_VQ_CBSIZE_LONG * MCR_DIM_SUBVEC];
extern const float mcr_codebook_8bit[MCR_VQ_CBSIZE_SHORT * MCR_DIM_SUBVEC];

// Mc ILD scalar codebook, 5bit, length 32
extern const float mcIldCodebook[MC_ILD_CBLEN];
extern const float mcInvIldCodebook[MC_ILD_CBLEN];

#endif
