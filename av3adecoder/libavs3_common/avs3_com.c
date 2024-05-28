#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "avs3_cnst_com.h"
#include "avs3_rom_com.h"
#include "avs3_prot_com.h"


short GetFrameLength(const long fs)
{
    short framelength = FRAME_LEN;

    switch (fs)
    {
    case AVS3_SAMPLING_48KHZ:
        framelength = FRAME_LEN;
        break;
    default:
        break;
    }

    return framelength;
}

char *toUpper(char *str)
{

    short i;

    i = 0;
    while (str[i] != '\0')
    {
        if (str[i] >= 'a' && str[i] <= 'z') str[i] += 'A' - 'a';
        i++;
    }

    return str;
}

// get mdct window, sine window
void GetMdctWindow(float* win, const short len)
{
    short i;
    for (i = 0; i < len; i++)
    {
        win[i] = (float)sin((AVS3_PI / (2.0f * len)) * (i + 0.5f));
    }

    return;
}

// get window function for each kind of transform
void GetWindowShape(AVS3_CORE_CONFIG_DATA_HANDLE hCoreConfig, const short transformType, float* winLeft, float*winRight)
{
    short i;
    if (transformType == ONLY_LONG_WINDOW)
    {
        Mvf2f(hCoreConfig->longWindow, winLeft, hCoreConfig->overlapLongSize);

        for (i = 0; i < hCoreConfig->overlapLongSize; i++)
        {
            winRight[i] = winLeft[hCoreConfig->overlapLongSize - i - 1];
        }
    }
    else if (transformType == ONLY_SHORT_WINDOW) 
    {
        Mvf2f(hCoreConfig->shortWindow, winLeft, hCoreConfig->overlapShortSize);

        for (i = 0; i < hCoreConfig->overlapShortSize; i++)
        {
            winRight[i] = winLeft[hCoreConfig->overlapShortSize - i - 1];
        }
    }
    else if (transformType == LONG_SHORT_TRANS_WINDOW) 
    {
        Mvf2f(hCoreConfig->longWindow, winLeft, hCoreConfig->overlapLongSize);

        for (i = 0; i < hCoreConfig->overlapShortSize; i++)
        {
            winRight[i] = hCoreConfig->shortWindow[hCoreConfig->overlapShortSize - i - 1];
        }
    }
    else if (transformType == SHORT_LONG_TRANS_WINDOW) 
    {
        Mvf2f(hCoreConfig->shortWindow, winLeft, hCoreConfig->overlapShortSize);

        for (i = 0; i < hCoreConfig->overlapLongSize; i++)
        {
            winRight[i] = hCoreConfig->longWindow[hCoreConfig->overlapLongSize - i - 1];
        }
    }
    else 
    {
        // Todo:
        assert("!Unknown window type!\n");
    }

    return;
}

void WindowSignal(
    AVS3_CORE_CONFIG_DATA_HANDLE hCoreConfig, 
    const float* signal, 
    float* winSignal, 
    const short transformType, 
    const float* winLeft,
    const float* winRight
)
{
    short i;
    const short overlapLong = hCoreConfig->overlapLongSize;
    const short overlapShort = hCoreConfig->overlapShortSize;
    const short paddingSize = hCoreConfig->overlapPaddingSize;

    if (transformType == ONLY_LONG_WINDOW)
    {
        /* left part */
        for (i = 0; i < overlapLong; i++)
        {
            winSignal[i] = winLeft[i] * signal[i];
        }

        /* right part */
        for (i = 0; i < overlapLong; i++)
        {
            winSignal[i + overlapLong] = winRight[i] * signal[i + overlapLong];
        }
    }
    else if (transformType == ONLY_SHORT_WINDOW)
    {
        /* left part */
        for (i = 0; i < overlapShort; i++)
        {
            winSignal[i] = winLeft[i] * signal[i];
        }

        /* right part */
        for (i = 0; i < overlapShort; i++)
        {
            winSignal[i + overlapShort] = winRight[i] * signal[i + overlapShort];
        }
    }
    else if (transformType == LONG_SHORT_TRANS_WINDOW)
    {
        /* left part */
        for (i = 0; i < overlapLong; i++)
        {
            winSignal[i] = winLeft[i] * signal[i];
        }

        /* middle flat */
        for (i = 0; i < paddingSize; i++)
        {
            winSignal[i + overlapLong] = signal[i+ overlapLong];
        }

        /* short part */
        for (i = 0; i < overlapShort; i++) 
        {
            winSignal[i + overlapLong + paddingSize] = winRight[i] * signal[i + overlapLong + paddingSize];
        }

        /* zero padding part */
        for (i = 0; i < paddingSize; i++)
        {
            winSignal[i + overlapLong + paddingSize + overlapShort] = 0.f;
        }
    }
    else if (transformType == SHORT_LONG_TRANS_WINDOW)
    {
        /* zero padding part */
        for (i = 0; i < paddingSize; i++)
        {
            winSignal[i] = 0.f;
        }

        /* left part */
        for (i = 0; i < overlapShort; i++)
        {
            winSignal[i + paddingSize] = winLeft[i] * signal[i+ paddingSize];
        }

        /* middle flat */
        for (i = 0; i < paddingSize; i++) 
        {
            winSignal[i + paddingSize + overlapShort] = signal[i + paddingSize + overlapShort];
        }

        /* right part */
        for (i = 0; i < overlapLong; i++) 
        {
            winSignal[i + paddingSize + overlapShort + paddingSize] = winRight[i] * signal[i + paddingSize + overlapShort + paddingSize];
        }
    }
    else
    {
        assert("!Unknown window type!\n");
    }

    return;
}


// init core coder config
// current only window function
void InitCoreConfig(AVS3_CORE_CONFIG_DATA_HANDLE hCoreConfig, const short frameLength) 
{
    hCoreConfig->overlapLongSize = frameLength;
    hCoreConfig->overlapShortSize = frameLength / N_BLOCK_SHORT;
    hCoreConfig->overlapPaddingSize = BLOCK_PADDING_SIZE;

    GetMdctWindow(hCoreConfig->longWindow, hCoreConfig->overlapLongSize);
    GetMdctWindow(hCoreConfig->shortWindow, hCoreConfig->overlapShortSize);
}


unsigned long Avs3SynthOutput(float synth[MAX_CHANNELS][BLOCK_LEN_LONG], const short output_frame, const short n_channels, short *synth_out) 
{
    short i, n;
    short synth_loc[BLOCK_LEN_LONG];
    unsigned long noClipping = 0;

    /*-----------------------------------------------------------------*
     * float to integer conversion with saturation control
     *-----------------------------------------------------------------*/

    for (n = 0; n < n_channels; n++)
    {
        noClipping += MvFloat2Short(synth[n], synth_loc, output_frame);

        for (i = 0; i < output_frame; i++)
        {
            synth_out[i*n_channels + n] = synth_loc[i];
        }
    }

    return noClipping;
}


void MdctSpectrumInterleave(
    float *mdctSpectrum,
    int16_t length,
    int16_t numShortBlock
)
{
    int16_t lenShortBlock = length / numShortBlock;
    float tmpSpectrum[FRAME_LEN] = {0.0f};

    for (int16_t i = 0; i < numShortBlock; i++) {
        for (int16_t j = 0; j < lenShortBlock; j++) {
            tmpSpectrum[i + numShortBlock * j] = mdctSpectrum[i * lenShortBlock + j];
        }
    }

    Mvf2f(tmpSpectrum, mdctSpectrum, length);
}


void MdctSpectrumDeinterleave(
    float *mdctSpectrum,
    int16_t length,
    int16_t numShortBlock
)
{
    int16_t lenShortBlock = length / numShortBlock;
    float tmpSpectrum[FRAME_LEN] = { 0.0f };

    for (int16_t i = 0; i < numShortBlock; i++) {
        for (int16_t j = 0; j < lenShortBlock; j++) {
            tmpSpectrum[i * lenShortBlock + j] = mdctSpectrum[i + numShortBlock * j];
        }
    }

    Mvf2f(tmpSpectrum, mdctSpectrum, length);
}


/*
Short window frame mdct spectrum grouping func
Get grouping info, including numGroups and groupIndicator
I/O params:
    float *mdctSpectrum             (i/o) mdct spectrum
    int16_t length                  (i)   spectrum length
    int16_t numShortBlock           (i)   number of short blocks in short win frame
    int16_t *groupIndicator         (o)   group indicator vector, 0 for transient, 1 for others
    int16_t *numGroups              (o)   number of groups for current frame
*/
static void GetGroupingInfo(
    float *mdctSpectrum,
    int16_t length,
    int16_t numShortBlock,
    int16_t *groupIndicator,
    int16_t *numGroups
)
{
    int16_t i, j;
    int16_t lenShortBlock = length / numShortBlock;         // length of short block
    float blockEner[NUM_BLOCKS] = { 0.0f };                 // block energy for each short window
    float maxBlockEner = 0.0f;                              // max block energy for 8 short windows
    float blockAvgEner = 0.0f;                              // averaged block energy
    int16_t numTransientBlock = 0;                          // number of transient blocks
    int16_t numOtherBlock = 0;                              // number of other blocks

    // block energy
    for (i = 0; i < numShortBlock; i++) {
        for (j = 0; j < lenShortBlock; j++) {
            blockEner[i] += mdctSpectrum[i * lenShortBlock + j] * mdctSpectrum[i * lenShortBlock + j];
        }
    }

    // avg block energy
    for (i = 0; i < numShortBlock; i++) {
        blockAvgEner += blockEner[i];
        if (maxBlockEner < blockEner[i]) {
            maxBlockEner = blockEner[i];
        }
    }
    // subtract max block energy from average calculation
    blockAvgEner = (blockAvgEner - maxBlockEner) / (numShortBlock - 1);

    // group indicator
    // decision based on block energy
    for (i = 0; i < numShortBlock; i++) {
        if (blockEner[i] > blockAvgEner * 1.5f) {
            groupIndicator[i] = 0;
            numTransientBlock += 1;
        }
        else {
            groupIndicator[i] = 1;
            numOtherBlock += 1;
        }
    }

    // if transient or other group has all blocks
    // set groupIndicator to all 0
    *numGroups = N_GROUP_SHORT_WIN;
    if (numTransientBlock == N_BLOCK_SHORT || numOtherBlock == N_BLOCK_SHORT) {

        numTransientBlock = N_BLOCK_SHORT;
        numOtherBlock = 0;

        *numGroups = 1;
        SetShort(groupIndicator, 0, N_BLOCK_SHORT);
    }

    return;
}


/*
Short window frame mdct spectrum grouping func
Perform spectrum grouping
I/O params:
    float *mdctSpectrum             (i/o) mdct spectrum
    int16_t length                  (i)   spectrum length
    int16_t numShortBlock           (i)   number of short blocks in short win frame
    int16_t *groupIndicator         (o)   group indicator vector, 0 for transient, 1 for others
*/
static void MdctSpectrumGrouping(
    float *mdctSpectrum,
    int16_t length,
    int16_t numShortBlock,
    int16_t *groupIndicator
)
{
    int16_t i, j;
    int16_t lenShortBlock = length / numShortBlock;         // length of short block
    float groupedSpec[FRAME_LEN] = { 0.0f };                // grouped mdct spec, one group for transient, one group for others
    float interleavedSpec[FRAME_LEN] = { 0.0f };            // interleaved mdct spec, interleave is done for each group
    int16_t groupIdx = 0;
    int16_t numTransientBlock = 0;                          // number of transient blocks
    int16_t numOtherBlock = 0;                              // number of other blocks
    int16_t offset;                                         // freq. bin offset for the second group

    // get number of transient and other blocks
    for (i = 0; i < numShortBlock; i++) {
        if (groupIndicator[i] == 0) {
            numTransientBlock++;
        }
        else {
            numOtherBlock++;
        }
    }

    // reform spectrum into group
    groupIdx = 0;
    // transient group
    for (i = 0; i < numShortBlock; i++) {

        if (groupIndicator[i] == 0) {

            for (j = 0; j < lenShortBlock; j++) {
                groupedSpec[groupIdx * lenShortBlock + j] = mdctSpectrum[i * lenShortBlock + j];
            }

            groupIdx++;
        }
    }
    // non-transient group
    for (i = 0; i < numShortBlock; i++) {

        if (groupIndicator[i] == 1) {

            for (j = 0; j < lenShortBlock; j++) {
                groupedSpec[groupIdx * lenShortBlock + j] = mdctSpectrum[i * lenShortBlock + j];
            }

            groupIdx++;
        }
    }

    // transient group interleave
    for (i = 0; i < numTransientBlock; i++) {
        for (j = 0; j < lenShortBlock; j++) {
            interleavedSpec[i + numTransientBlock * j] = groupedSpec[i * lenShortBlock + j];
        }
    }
    // non-transient group interleave
    offset = numTransientBlock * lenShortBlock;
    for (i = 0; i < numOtherBlock; i++) {
        for (j = 0; j < lenShortBlock; j++) {
            interleavedSpec[offset + i + numOtherBlock * j] = groupedSpec[offset + i * lenShortBlock + j];
        }
    }

    Mvf2f(interleavedSpec, mdctSpectrum, length);

    return;
}


/*
Short window frame mdct spectrum degrouping func
I/O params:
    float *mdctSpectrum             (i/o) mdct spectrum
    int16_t length                  (i)   spectrum length
    int16_t numShortBlock           (i)   number of short blocks in short win frame
    int16_t *groupIndicator         (i)   group indicator vector, 0 for transient, 1 for others
*/
static void MdctSpectrumDegrouping(
    float *mdctSpectrum,
    int16_t length,
    int16_t numShortBlock,
    int16_t *groupIndicator
)
{
    int16_t i, j;
    int16_t lenShortBlock = length / numShortBlock;     // length of short block
    float groupedSpec[FRAME_LEN] = { 0.0f };            // grouped mdct spec, one group for transient, one group for others
    float tmpSpec[FRAME_LEN] = { 0.0f };                // deinterleaved mdct spec
    int16_t groupIdx = 0;
    int16_t numTransientBlock = 0;                      // number of transient blocks
    int16_t numOtherBlock = 0;                          // number of other blocks
    int16_t offset;                                     // freq. bin offset for the second group

    // get number of transient and other blocks
    for (i = 0; i < numShortBlock; i++) {
        if (groupIndicator[i] == 0) {
            numTransientBlock++;
        }
        else {
            numOtherBlock++;
        }
    }

    // transient group deinterleave
    for (i = 0; i < numTransientBlock; i++) {
        for (j = 0; j < lenShortBlock; j++) {
            groupedSpec[i * lenShortBlock + j] = mdctSpectrum[i + numTransientBlock * j];
        }
    }
    // non-transient group interleave
    offset = numTransientBlock * lenShortBlock;
    for (i = 0; i < numOtherBlock; i++) {
        for (j = 0; j < lenShortBlock; j++) {
            groupedSpec[offset + i * lenShortBlock + j] = mdctSpectrum[offset + i + numOtherBlock * j];
        }
    }

    // reform spectrum from group to normal
    groupIdx = 0;
    // transient group
    for (i = 0; i < numShortBlock; i++) {

        if (groupIndicator[i] == 0) {

            for (j = 0; j < lenShortBlock; j++) {
                tmpSpec[i * lenShortBlock + j] = groupedSpec[groupIdx * lenShortBlock + j];
            }

            groupIdx++;
        }
    }
    // non-transient group
    for (i = 0; i < numShortBlock; i++) {

        if (groupIndicator[i] == 1) {

            for (j = 0; j < lenShortBlock; j++) {
                tmpSpec[i * lenShortBlock + j] = groupedSpec[groupIdx * lenShortBlock + j];
            }

            groupIdx++;
        }
    }

    Mvf2f(tmpSpec, mdctSpectrum, length);
}


/*
Top level interface for grouping
I/O params:
    float *mdctSpectrum             (i/o) mdct spectrum
    int16_t length                  (i)   spectrum length
    int16_t transformType           (i)   transform type of current frame
    int16_t *groupIndicator         (o)   group indicator vector, 0 for transient, 1 for others
    int16_t *numGroups              (o)   number of groups for current frame
*/
void SpectrumGroupingEnc(
    float *mdctSpectrum,
    int16_t length,
    int16_t transformType,
    int16_t *groupIndicator,
    int16_t *numGroups
)
{
    if (transformType == ONLY_SHORT_WINDOW) {
        // short window
        // deinterleave
        MdctSpectrumDeinterleave(mdctSpectrum, length, N_BLOCK_SHORT);
        // get numGroups and groupIndicator, then grouping
        GetGroupingInfo(mdctSpectrum, length, N_BLOCK_SHORT, groupIndicator, numGroups);
        MdctSpectrumGrouping(mdctSpectrum, length, N_BLOCK_SHORT, groupIndicator);
    }
    else {
        // long window and transition window
        // set numGroups to 1, clear groupIndicator
        *numGroups = 1;
        SetShort(groupIndicator, 0, N_BLOCK_SHORT);
    }
}


/*
Top level interface for degrouping
I/O params:
    float *mdctSpectrum             (i/o) mdct spectrum
    int16_t length                  (i)   spectrum length
    int16_t transformType           (i)   transform type of current frame
    int16_t *groupIndicator         (i)   group indicator vector, 0 for transient, 1 for others
*/
void SpectrumDegroupingDec(
    float *mdctSpectrum,
    int16_t length,
    int16_t transformType,
    int16_t *groupIndicator
)
{
    if (transformType == ONLY_SHORT_WINDOW) {
        // short window
        // degrouping and interleave, degrouping is based on groupIndicator
        MdctSpectrumDegrouping(mdctSpectrum, length, N_BLOCK_SHORT, groupIndicator);
        MdctSpectrumInterleave(mdctSpectrum, length, N_BLOCK_SHORT);
    }
}


/*
Common function for all modes to get available bits before QC
I/O params:
    short bitsPerFrame              (i) num of bits per frame
    short bitsUsed                  (i) num of bits already used
    short *numGroups                (i) num of groups in each channel
    short nChans                    (i) num of channels in QC
    ret                             (o) available bits for QC
*/
int32_t GetAvailableBits(
    int32_t bitsPerFrame,
    int32_t bitsUsed,
    short *numGroups,
    short nChans,
    NnTypeConfig nnTypeConfig
)
{
    int32_t availableBits = 0;

    if (nnTypeConfig == NN_TYPE_DEFAULT_MAIN) {
        availableBits = bitsPerFrame - bitsUsed -
            nChans * (NBITS_IS_FEAT_AMPLIFIED + NBITS_FEATURE_SCALE + NBITS_CONTEXT_NUM_BYTES);
    }
    else if (nnTypeConfig == NN_TYPE_DEFAULT_LC) {
        availableBits = bitsPerFrame - bitsUsed -
            nChans * (NBITS_FEATURE_SCALE_LC + NBITS_CONTEXT_NUM_BYTES);
    }

    for (int16_t i = 0; i < nChans; i++) {

        if (numGroups[i] == 1) {
            // long window, or short window, one group
            availableBits -= NBITS_NF_PARAM;
        }
        else if (numGroups[i] == N_GROUP_SHORT_WIN) {
            // short window, two groups
            availableBits -= N_GROUP_SHORT_WIN * NBITS_NF_PARAM;
        }
    }

    return availableBits;
}


// crc16 check function
uint16_t Crc16(
    uint8_t * bitstream, 
    uint32_t bytesFrame
)
{
    uint16_t oldCrc16;
    uint16_t crc16;
    uint8_t t;

    oldCrc16 = 0xFFFF;

    while (bytesFrame--)
    {
        t = (uint8_t)((oldCrc16 >> 8) & 0xFF);
        oldCrc16 = (oldCrc16 << 8) | *bitstream++; 
        oldCrc16 = oldCrc16 ^ crc16Table[t];
    }

    crc16 = oldCrc16;

    return crc16;
}
