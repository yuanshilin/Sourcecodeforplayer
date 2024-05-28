#include <stdlib.h>
#include <math.h>

#include "avs3_debug.h"
#include "avs3_cnst_com.h"
#include "avs3_prot_com.h"
#include "avs3_prot_enc.h"
#include "Version.h"

static void Avs3EncoderConfig(AVS3EncoderHandle stAvs3)
{
    if(stAvs3->avs3CodecFormat == AVS3_HOA_FORMAT)
    {
        stAvs3->numChansInput = stAvs3->hEncHoa->hHoaConfig->nTotalChansInput;
    }

    return;
}

// pre-analysis for core coder
// including transient detection and windowing/transform
void Avs3PreAnalysis(AVS3EncoderHandle stAvs3, const short nChans, const short LenFrame)
{
    short ch;
    AVS3_ENC_CORE_HANDLE hEncCore = NULL;

    /* Transient Detection */
    for (ch = 0; ch < nChans; ch++)
    {
        hEncCore = stAvs3->hEncCore[ch];
        hEncCore->transformType = WindowTypeDetect(&hEncCore->winTypeDetector, (hEncCore->lookahead - FRAME_LEN / 2),
            hEncCore->frameLength, stAvs3->initFrame);
    }

    /* Time Domain to Frequency Domain */
    CoreSignalAnalysis(stAvs3, nChans, LenFrame);

    /* Grouping mdct spectrum in short window frame */
    for (ch = 0; ch < nChans; ch++)
    {
        hEncCore = stAvs3->hEncCore[ch];
        if (hEncCore->transformType == ONLY_SHORT_WINDOW) 
        {
            MdctSpectrumInterleave(hEncCore->origSpectrum, hEncCore->frameLength, N_BLOCK_SHORT);
        }
    }

    /* Get silence flag for mc and mix mode */
    if (stAvs3->avs3CodecFormat == AVS3_MC_FORMAT && stAvs3->enableSilDetect == 1) {
        McMixGetSilenceFlag(stAvs3, nChans, LenFrame);
    }

    /* Clear HF mdct lines for LFE channel in MC mode */
    if (stAvs3->avs3CodecFormat == AVS3_MC_FORMAT || 
        stAvs3->avs3CodecFormat == AVS3_MIX_FORMAT) {
        for (ch = 0; ch < nChans; ch++) {
            // has lfe and lfe is current channel
            if (stAvs3->hMcEnc->lfeExist && stAvs3->hMcEnc->lfeChIdx == ch) {
                hEncCore = stAvs3->hEncCore[ch];
                McLfeProc(hEncCore->origSpectrum);
            }
        }
    }

    /* LPC analysis and spectrum shaping */
    for (ch = 0; ch < nChans; ch++) {
        Avs3FdSpectrumShaping(stAvs3->hEncCore[ch], ch);
    }

    /* temporal noise shaping */
    for (ch = 0; ch < nChans; ch++) {

        hEncCore = stAvs3->hEncCore[ch];
        TnsEnc(&hEncCore->tnsData, hEncCore->origSpectrum, hEncCore->transformType == ONLY_SHORT_WINDOW);
    }

    /* Encoder side bwe */
    for (ch = 0; ch < nChans; ch++) {

        hEncCore = stAvs3->hEncCore[ch];

        if (hEncCore->bwePresent == 1) {
            BweApplyEnc(&hEncCore->bweConfig, &hEncCore->bweEncData, hEncCore->origSpectrum, NULL,
                hEncCore->transformType == ONLY_LONG_WINDOW);
        }
    }

    /* Write core side bits into bitstream */
    /* Including window type, fd shaping, td shaping and BWE */
    WriteCoreSideBitstream(stAvs3, nChans, stAvs3->bitstream, &stAvs3->totalSideBits);
}


// QC function
void Avs3Qc(
    AVS3EncoderHandle stAvs3,
    short *target,
    const short nChans
)
{
    short ch;
    AVS3_ENC_CORE_HANDLE hEncCore = NULL;

    float featureIn[FRAME_LEN][2];          // 2D feature map for neural qc

    /* Neural QC process */
    for (ch = 0; ch < nChans; ch++) {

        hEncCore = stAvs3->hEncCore[ch];

        // copy feature to 2D feature map
        for (int16_t i = 0; i < FRAME_LEN; i++) {
            featureIn[i][0] = hEncCore->origSpectrum[i];
            featureIn[i][1] = 0.0f;
        }

        // get number of spectral lines for NF calculation
        int16_t numLinesNonZero = 0;
        if (hEncCore->bwePresent) {
            numLinesNonZero = hEncCore->bweConfig.bweStartLine;
        }
        else {
            numLinesNonZero = hEncCore->frameLength;
        }

        // Init neural QC data structure
        InitNeuralQcData(&hEncCore->neuralQcData);

        // MDCT QC process
        if (stAvs3->nnTypeConfig == NN_TYPE_DEFAULT_MAIN) {
            MdctQuantEncodeHyper(stAvs3->baseCodecSt, stAvs3->contextCodecSt, &hEncCore->neuralQcData,
                featureIn, hEncCore->frameLength, 1, numLinesNonZero, hEncCore->numGroups, hEncCore->groupIndicator, target[ch]);
        }
        else if (stAvs3->nnTypeConfig == NN_TYPE_DEFAULT_LC) {
            MdctQuantEncodeHyperLc(stAvs3->baseCodecSt, stAvs3->contextCodecSt, &hEncCore->neuralQcData,
                featureIn, hEncCore->frameLength, 1, numLinesNonZero, hEncCore->numGroups, hEncCore->groupIndicator, &hEncCore->preFeatureScale, target[ch]);
        }

        // post proc for nf param, only for low bitrate, harmonic or tone signals
        // only for short window
        if (hEncCore->transformType != ONLY_SHORT_WINDOW) {
            NfParamPostProc(&hEncCore->neuralQcData, hEncCore->origSpectrum, numLinesNonZero, stAvs3->totalBitrate, nChans);
        }
    }

    // write QC bitstream for all channels
    WriteQcBitstream(stAvs3, nChans, stAvs3->bitstream, &stAvs3->totalSideBits);

    return;
}


// frame level core coding function
void Avs3CoreEncode(AVS3EncoderHandle stAvs3, float data[MAX_CHANNELS][MAX_FRAME_LEN], const short lenFrame, const short nChans)
{
    short ch;
    const short offset = stAvs3->initFrame ? 0 : stAvs3->lookaheadSamples;
    const short delay = stAvs3->initFrame ? stAvs3->lookaheadSamples : 0;

    short target[MAX_CHANNELS] = { 0 };

    AVS3_ENC_CORE_HANDLE hEncCore = NULL;

    int16_t nChansQc = nChans;              // number channels for QC

    // copy input data to core buffer
    for (ch = 0; ch < nChans; ch++)
    {
        Mvf2f(data[ch], stAvs3->hEncCore[ch]->inputSignal + offset, lenFrame + delay);
    }

    /* Transient Detection & Windowing Signal */
    Avs3PreAnalysis(stAvs3, nChans, lenFrame);

    if (stAvs3->avs3CodecFormat == AVS3_MONO_FORMAT)
    {
        Avs3MonoEncoder(stAvs3, target);
    }
    else if (stAvs3->avs3CodecFormat == AVS3_STEREO_FORMAT)
    {
        if (stAvs3->hMdctStereo->useMcr == 0) {
            // stereo ms decision, downmix and bit split
            Avs3StereoEncoder(stAvs3, target);
        }
        else {
            // MCR stereo, number channels for QC is 1
            Avs3StereoMcrEncoder(stAvs3, target);
            nChansQc = 1;
        }
    }
    else if (stAvs3->avs3CodecFormat == AVS3_MC_FORMAT)
    {
        Avs3McEncoder(stAvs3, target);
    }
    else if (stAvs3->avs3CodecFormat == AVS3_HOA_FORMAT)
    {
        /* HOA down-mix and bits split */
        Avs3HoaCoreEncoder(stAvs3, target);
    }
    else if (stAvs3->avs3CodecFormat == AVS3_MIX_FORMAT)
    {
        Avs3MixEncoder(stAvs3, target);
    }
    else
    {
        // Todo
    }

    /* Neural QC process */
    Avs3Qc(stAvs3, target, nChansQc);

    /* Update input signal */
    for (ch = 0; ch < nChans; ch++)
    {
        Mvf2f(stAvs3->hEncCore[ch]->inputSignal, stAvs3->hEncCore[ch]->signalBuffer, lenFrame);
        Mvf2f(stAvs3->hEncCore[ch]->lookahead, stAvs3->hEncCore[ch]->inputSignal, stAvs3->hEncCore[ch]->lookaheadSamples);

        stAvs3->hEncCore[ch]->lastTransformType = stAvs3->hEncCore[ch]->transformType;
    }

    return;
}


// top level frame level encoding function
void Avs3Encode(AVS3EncoderHandle stAvs3, const short *data, const short samples)
{
    short i, ch;
    short nChans;
    short nSamplesPerChan;
    short inputFrameLength;
    float dataFloat[MAX_CHANNELS][MAX_FRAME_LEN];

    Avs3EncoderConfig(stAvs3);

    nChans = stAvs3->numChansInput;
    nSamplesPerChan = samples / nChans;
    inputFrameLength = stAvs3->frameLength;

    // short to float
    for (ch = 0; ch < nChans; ch++)
    {
        for (i = 0; i < nSamplesPerChan; i++)
        {
            dataFloat[ch][i] = (float)data[i*nChans + ch];
        }
    }
    // padding zeros if not sufficient data
    if (nSamplesPerChan < inputFrameLength)
    {
        for (ch = 0; ch < nChans; ch++)
        {
            SetZero(dataFloat[ch] + nSamplesPerChan, inputFrameLength - nSamplesPerChan);
        }
    }

    // HOA processing, before core coding
    if (stAvs3->avs3CodecFormat == AVS3_HOA_FORMAT)
    {
        Avs3HOAEncoder(stAvs3, dataFloat, inputFrameLength);

        Avs3HOAReconfig(stAvs3, &nChans);
    }

    // core coding
    Avs3CoreEncode(stAvs3, dataFloat, inputFrameLength, nChans);

    return;
}