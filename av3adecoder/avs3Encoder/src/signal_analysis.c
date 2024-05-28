#include<stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "avs3_options.h"
#include "avs3_debug.h"
#include "avs3_prot_com.h"
#include "avs3_prot_enc.h"


void Avs3LocalDecoder(AVS3_ENC_CORE_HANDLE hEncCore, float output[BLOCK_LEN_LONG])
{
    AVS3_CORE_CONFIG_DATA_HANDLE hCoreConfig = hEncCore->hCoreConfig;

    float winLeft[BLOCK_LEN_LONG];
    float winRight[BLOCK_LEN_LONG];
    float tdaSiganl[BLOCK_LEN_LONG + BLOCK_LEN_LONG];
    short overlapSize;

    SetZero(tdaSiganl, BLOCK_LEN_LONG + BLOCK_LEN_LONG);

    Mvf2f(hEncCore->origSpectrum, tdaSiganl, BLOCK_LEN_LONG);

    if (hEncCore->transformType != ONLY_SHORT_WINDOW)
    {
        overlapSize = hCoreConfig->overlapLongSize;

        /* Inverse MDCT */
        IMDCT(tdaSiganl, 2 * overlapSize);

        /* Get window shape */
        GetWindowShape(hCoreConfig, hEncCore->transformType, winLeft, winRight);

        /* Window signal */
        WindowSignal(hCoreConfig, tdaSiganl, tdaSiganl, hEncCore->transformType, winLeft, winRight);

        /* Overlap-add */
        Vadd(tdaSiganl, hEncCore->synthBuffer, tdaSiganl, overlapSize);

        /* however current frame is a transition frame or long window, stored all the half frame including zero padding part and flat part, it make synthesis easily */
        Mvf2f(tdaSiganl + overlapSize, hEncCore->synthBuffer, overlapSize);

        /* Output */
        Mvf2f(tdaSiganl, output, overlapSize);
    }
    else 
    {
        float tmpSynthBuffer[BLOCK_LEN_SHORT];
        float winShort[BLOCK_LEN_SHORT + BLOCK_LEN_SHORT];
        float tmpSynth[FRAME_LEN];
        const short synthOffset = hCoreConfig->overlapPaddingSize;

        overlapSize = hCoreConfig->overlapShortSize;

        SetZero(tmpSynth, FRAME_LEN);

        /* get last frame overlap add buffer for the first short block */
        Mvf2f(hEncCore->synthBuffer + synthOffset, tmpSynthBuffer, overlapSize);
        
        /* get window shape */
        GetWindowShape(hCoreConfig, hEncCore->transformType, winLeft, winRight);

        /* loop through blocks */
        for (short block = 0; block < N_BLOCK_SHORT; block++)
        {
            SetZero(winShort, 2 * overlapSize);

            Mvf2f(tdaSiganl + block * overlapSize, winShort, overlapSize);

            /* Inverse MDCT */
            IMDCT(winShort, 2 * overlapSize);

            /* Windowing left part */
            VMult(winShort, winLeft, winShort, overlapSize);

            /* Windowing right part */
            VMult(winShort + overlapSize, winRight, winShort + overlapSize, overlapSize);

            /* overlap add */
            Vadd(winShort, tmpSynthBuffer, winShort, overlapSize);

            /* update inter-frame synthesis buffer */
            Mvf2f(winShort + overlapSize, tmpSynthBuffer, overlapSize);
            
            /* Output */
            Mvf2f(winShort, tmpSynth + block * overlapSize, overlapSize);
        }
        
        /* output part 1 (last frame) */
        Mvf2f(hEncCore->synthBuffer, output, synthOffset);

        /* output part 2 (current frame )*/
        Mvf2f(tmpSynth, output + synthOffset, hEncCore->frameLength - synthOffset);

        /* synthesis buffer part 1 */
        Mvf2f(tmpSynth + hEncCore->frameLength - synthOffset, hEncCore->synthBuffer, synthOffset);

        /* synthesis buffer part 2 */
        Mvf2f(tmpSynthBuffer, hEncCore->synthBuffer + synthOffset, overlapSize);

        /* synthesis buffer part 3 (zero padding part) */
        SetZero(hEncCore->synthBuffer + synthOffset + overlapSize, hEncCore->frameLength - (synthOffset + overlapSize));
    }

    return;
}


// signal analysis for core coder
// including 
void CoreSignalAnalysis(AVS3EncoderHandle stAvs3, const short nChans, const short lenFrame)
{
    short ch;
    float winLeft[BLOCK_LEN_LONG];
    float winRight[BLOCK_LEN_LONG];
    float mdctWin[BLOCK_LEN_LONG + BLOCK_LEN_LONG];
    float mdctWinShort[BLOCK_LEN_SHORT + BLOCK_LEN_SHORT];
    float* signalInput = NULL;
    short overlapSize;


    AVS3_ENC_CORE_HANDLE hEncCore = NULL;
    AVS3_CORE_CONFIG_DATA_HANDLE hCoreConfig = NULL;

    SetZero(mdctWin, BLOCK_LEN_LONG + BLOCK_LEN_LONG);

    for (ch = 0; ch < nChans; ch++) 
    {
        hEncCore = stAvs3->hEncCore[ch];
        hCoreConfig = hEncCore->hCoreConfig;

        GetWindowShape(hCoreConfig, hEncCore->transformType, winLeft, winRight);

        if (hEncCore->transformType != ONLY_SHORT_WINDOW)
        {
            /* input signal */
            signalInput = hEncCore->signalBuffer;

            overlapSize = hCoreConfig->overlapLongSize;

            /* Windowing signal */
            WindowSignal(hCoreConfig, signalInput, mdctWin, hEncCore->transformType, winLeft, winRight);

            /* Copy for LPC */
            Mvf2f(mdctWin, hEncCore->winSignal48kHz, 2 * overlapSize);

            MDCT(mdctWin, hEncCore->origSpectrum, 2 * overlapSize);
        }
        else 
        {
            /* input signal with padding offset */
            signalInput = hEncCore->signalBuffer + hCoreConfig->overlapPaddingSize;
            
            overlapSize = hCoreConfig->overlapShortSize;

            for (short block = 0; block < N_BLOCK_SHORT; block++) 
            {
                /* Windowing short block signal */
                WindowSignal(hCoreConfig, signalInput, mdctWinShort, hEncCore->transformType, winLeft, winRight);

                /* Copy for LPC */
                Mvf2f(mdctWinShort, hEncCore->winSignal48kHz + block * (2 * overlapSize), 2 * overlapSize);

                MDCT(mdctWinShort, hEncCore->origSpectrum + block * overlapSize, 2 * overlapSize);

                /* set new input signal start offset */
                signalInput += overlapSize;
            }
        }
    }
}
