
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_stat_com.h"
#include "avs3_rom_com.h"
#include "avs3_prot_com.h"


/*
Calculate energy ratio between channels
I/O params:
    float *x0 and float *x1                   (i) input spetrum for each channel
    const short frameLength                   (i) input frame length
*/
float CalculateEnergyRatio(
    float *x0,
    float *x1,
    const short frameLengh
)
{
    float energy[STEREO_CHANNELS] = { 0.0f };

    for (int i = 0; i < frameLengh; i++) {
        energy[0] += x0[i] * x0[i];
        energy[1] += x1[i] * x1[i];
    }

    energy[0] = (float)sqrt(energy[0]);
    energy[1] = (float)sqrt(energy[1]);

    if ((energy[0] + energy[1]) > 0) {
        return energy[0] / (energy[0] + energy[1]);
    }
    else {
        return -1.0f;
    }
}


/*
Compute bits ratio between channels
I/O params:
    float x0[] and float x1[]                 (i) inputspetrum for each channel
    const short frameLength                   (i) input frame length
    const short isMs                          (i) ms flag
    short *bitsRatio                          (o) bit split ratio
*/
void ComputeBitsRatio(
    float *x0, 
    float *x1, 
    const short frameLength, 
    const short isMs,
    short *bitsRatio
)
{
    float localRatio = CalculateEnergyRatio(x0, x1, frameLength);

    *bitsRatio = EQUAL_BITS_RATIO;

    if (localRatio >= 0)
    {
        *bitsRatio = (unsigned short)(BITS_SPLIT_RANGE * localRatio + 0.5f);

        // adjust bit split ratio when ms off
        if (isMs == 0) {
            if (*bitsRatio <= 3) {
                *bitsRatio += 1;
            }
            else if (*bitsRatio >= BITS_SPLIT_RANGE - 3) {
                *bitsRatio -= 1;
            }
        }

        // limit range of bit split ratio
        *bitsRatio = min(BITS_SPLIT_RANGE - 1, max(1, *bitsRatio));
    }
}


/*
Bits Allocation function
I/O params:
    short availableBits         (i) available bits in current frame
    short bitsRatio             (i) bit split ratio
    short *channelBytes         (o) Q/C target bytes
*/
void StereoBitsAllocation(
    short availableBits,
    short bitsRatio,
    short *channelBytes
)
{
    short availableBytes = 0;

    availableBytes = (short)floor((float)availableBits / 8.0f);

    channelBytes[0] = bitsRatio * (short)floor((float)availableBytes / BITS_SPLIT_RANGE);
    channelBytes[1] = availableBytes - channelBytes[0];
}


/* 
MS process for stereo mode, including ILD calculation
I/O params:
    float *x0                   (i/o) L channel mdct spectrum
    float *x1                   (i/o) R channel mdct spectrum
    const short frameLength     (i)   frame length
    short *ild                  (o)   ILD param
*/
void StereoMsProcess(
    float *x0,
    float *x1,
    const short frameLength,
    short *ild
)
{
    float energyRatio = 0.0f;
    const float a = 0.5f * (float)sqrt(2.0f);
    float tmp;

    // calcualte energy relation between channels
    energyRatio = CalculateEnergyRatio(x0, x1, frameLength);

    // ild quantization
    *ild = max(1, min(((short)(ENERGY_BALENCE_RANGE * energyRatio + 0.5f)), ENERGY_BALENCE_RANGE - 1));

    // energy ratio for channel
    energyRatio = (float)ENERGY_BALENCE_RANGE / *ild - 1;

    // energy balance
    if (energyRatio > 1.0f) {
        VMultC(x1, 1.0f / energyRatio, x1, frameLength);
    }
    else if (energyRatio < 1.0f) {
        VMultC(x0, energyRatio, x0, frameLength);
    }

    // MS process
    for (short i = 0; i < frameLength; i++)
    {
        tmp = x0[i];
        x0[i] = a * (x0[i] + x1[i]);
        x1[i] = a * (tmp - x1[i]);
    }

    return;
}


/*
Inverse MS process for stereo mode, including inverse ild
I/O params:
    float *x0                   (i/o) L channel mdct spectrum
    float *x1                   (i/o) R channel mdct spectrum
    const short frameLength     (i)   frame length
    short ild                   (i)   ILD param
*/
void StereoInvMsProcess(
    float *x0,
    float *x1,
    const short frameLength,
    short ild
)
{
    float energyRelation = 0.0f;
    const float a = 0.5f * (float)sqrt(2.0f);
    float tmp;

    // inverse MS process
    for (short i = 0; i < frameLength; i++)
    {
        tmp = x0[i];
        x0[i] = a * (x0[i] + x1[i]);
        x1[i] = a * (tmp - x1[i]);
    }

    // energy ratio for channel
    energyRelation = (float)ENERGY_BALENCE_RANGE / ild - 1;

    if (energyRelation > 1.0f) {
        VMultC(x1, energyRelation, x1, frameLength);
    }
    else if (energyRelation < 1.0f) {
        VMultC(x0, 1.0f / energyRelation, x0, frameLength);
    }

    return;
}
