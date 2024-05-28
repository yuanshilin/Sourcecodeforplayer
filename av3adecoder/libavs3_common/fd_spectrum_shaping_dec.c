
#include <stdlib.h>
#include <stdio.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_rom_com.h"
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"


/*
Inverse spectrum shaping
*/
void Avs3FdInvSpectrumShaping(
    short *lsfVqIndex,
    float *mdctSpectrum,
    short lsfLbrFlag
)
{
    short len;

    float lpcGain[BLOCK_LEN_LONG] = { 0.0f };
    float lsfQ[LPC_ORDER] = { 0.0f };
    float lspQ[LPC_ORDER] = { 0.0f };
    float lpcQ[LPC_ORDER + 1] = { 0.0f };

    len = BLOCK_LEN_LONG + BLOCK_LEN_LONG;

    /* LSF dequantization */
    LsfQuantDec(lsfVqIndex, lsfQ, LPC_ORDER, lsfLbrFlag);

    /* compute quantized LSP parameters */
    LsfToLsp(lsfQ, lspQ, LPC_ORDER, AVS3_SAMPLING_48KHZ);

    /* compute quantized LP parameters using quantized LSP parameters */
    LspToLpc(lspQ, lpcQ, LPC_ORDER);

    /* inverse spectrum shaping */
    SpectrumShaping(mdctSpectrum, lpcQ, lpcGain, LPC_ORDER, len, 0);

    return;
}
