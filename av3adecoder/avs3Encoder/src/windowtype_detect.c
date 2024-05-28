/*************************************************************************
        (C) Copyright Huawei
**************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_cnst_enc.h"
#include "avs3_rom_com.h"
#include "avs3_stat_enc.h"
#include "avs3_prot_com.h"
#include "avs3_prot_enc.h"


// Local functions
static void HpFilter(float hpHistory[], float const * inPut, const short frameLength, float * outPut);
static void CalculateBlockEnergies(const float  * inPut, WindowTypeDetectData *winTypeDetector, const short frameLength);
static int16_t CalculateWindowType(WindowTypeDetectData *winTypeDetector, const short initFrame);


/*
FIR high pass filter function
I/O params:
    float hpHistory[]        (i) history data
    float const * inPut      (i) input raw data
    const short frameLength  (i) input data length
    float * outPut           (o) output high pass filtered data
*/
static void HpFilter(
    float hpHistory[], 
    float const *inPut, 
    const short frameLength, 
    float * outPut
) 
{
    short i;
    float inPutAppend[FRAME_LEN + HP_ORDER];

    // update inPutAppend buffer
    for (i = 0; i < HP_ORDER; i++) {
        inPutAppend[i] = hpHistory[i];
    }
    for (i = HP_ORDER; i < frameLength + HP_ORDER; i++) {
        inPutAppend[i] = inPut[i - HP_ORDER];
    }

    // hp filter
    for (i = HP_ORDER; i < frameLength + HP_ORDER; i++) {
        for (short j = 0; j < HP_ORDER + 1; j++) {
            outPut[i  - HP_ORDER] += hpFilterCoff[j] * inPutAppend[i -j];
        }
    }

    // update hpHistory
    for (i = 0; i < HP_ORDER; i++) {
        hpHistory[i] = inPut[frameLength - HP_ORDER + i];
    }
}


/*
Calculate block energy and threshold energy for each block
I/O params:
   float const * inPut                      (i)   input data
   WindowTypeDetectData *winTypeDetector    (i/o) window type detection structure data
   const short frameLength                  (i)   frame length
*/
static void CalculateBlockEnergies(
    float const * inPut, 
    WindowTypeDetectData *winTypeDetector, 
    const short frameLength
) 
{
    short i;
    short nblockSize = winTypeDetector->blockSize;
    float *blockEnergy = winTypeDetector->blockEnergy;
    float *thresholdEnergy = winTypeDetector->thresholdEnergy;
    float *thresholdEnergyHis = &(winTypeDetector->thresholdEnergy[NUM_BLOCKS]);

    // calculate block energies
    SetFloat(winTypeDetector->blockEnergy, MIN_ENERGY, NUM_BLOCKS);
    for (i = 0; i < NUM_BLOCKS; i++) {
        for (short j = 0; j < nblockSize; j++) {
            blockEnergy[i] += inPut[i * nblockSize + j]  *  inPut[i * nblockSize + j];
        }
    }

    // calculate block thresholdEnergy
    SetFloat(winTypeDetector->thresholdEnergy, MIN_ENERGY, NUM_BLOCKS);
    for (i = 0; i < NUM_BLOCKS; i++) {
        thresholdEnergy[i] = *thresholdEnergyHis;

        *thresholdEnergyHis = (*thresholdEnergyHis) * ATTENUATION_COEFF;

        if (blockEnergy[i] > (*thresholdEnergyHis)) {
            *thresholdEnergyHis = blockEnergy[i];
        }
    }
}


/*
Calculate window type
I/O params:
   WindowTypeDetectData *winTypeDetector     (i/o) window type detection structure data
   const short initFrame                     (i)  first frame flag
*/
static int16_t CalculateWindowType(
    WindowTypeDetectData *winTypeDetector, 
    const short initFrame
) 
{
    int16_t windowType;

    //transitent and its postion eatimate
    float *blockEnergy = winTypeDetector->blockEnergy;          // block energy buffer pointer
    float *thresholdEnergy = winTypeDetector->thresholdEnergy;  // threshold energy buffer pointer
    int16_t curIsTransient = 0;                                 // flag for current frame
    int16_t *preIsTransient = winTypeDetector->preIsTransient;  // pointer to transient history buffer

    // first frame, no transient
    if (initFrame) {
        curIsTransient = 0;
    }
    else {
        // following frames, detect energy change
        for (short i = 0; i < NUM_BLOCKS; i++) {
            if (blockEnergy[i] > TH_ENERGY_COEFF * thresholdEnergy[i]) {
                curIsTransient = 1;
                break;
            }
        }
    }
    
    //window type eatimate
    if (curIsTransient == 0 && preIsTransient[1] == 0 && preIsTransient[0] == 0) {
        windowType = ONLY_LONG_WINDOW;
    }
    else if (curIsTransient == 1 && preIsTransient[1] == 0 && preIsTransient[0] == 0) {
        windowType = LONG_SHORT_TRANS_WINDOW;
    }
    else if (curIsTransient == 0 && preIsTransient[1] == 0 && preIsTransient[0] == 1) {
        windowType = SHORT_LONG_TRANS_WINDOW;
    }
    else {
        windowType = ONLY_SHORT_WINDOW;
    }

    //update transitent history
    preIsTransient[0] = preIsTransient[1];
    preIsTransient[1] = curIsTransient;
    
    return windowType;
}


 /*
 Init window type detection
 I/O params:
    const short frameLength                   (i) input frame length
    WindowTypeDetectData *winTypeDetector     (o) window type detection structure data
 */
void InitWindowTypeDetect(
    const short frameLength, 
    WindowTypeDetectData *winTypeDetector
) 
{
    // block size of detector
    if (frameLength % NUM_BLOCKS != 0) {
		LOGD("Error frameLength\n");
        exit(-1);
    }
    winTypeDetector->blockSize = frameLength / NUM_BLOCKS;

    // init energy buffer
    SetFloat(winTypeDetector->blockEnergy, MIN_ENERGY, NUM_BLOCKS);
    SetFloat(winTypeDetector->thresholdEnergy, MIN_ENERGY, NUM_BLOCKS + 1);

    // init high pass history
    SetZero(winTypeDetector->hpHistory, HP_ORDER);

    // init transitent state history
    winTypeDetector->preIsTransient[0] = 0;
    winTypeDetector->preIsTransient[1] = 0;
}


/*
Window type detector Interface function
I/O parames:
    WindowTypeDetectData *winTypeDetector   (i/o)  window type detecttion structure data
    const short frameLength                 (i)    input data length
    const short initFrame                   (i)    first frame flag
*/
int16_t WindowTypeDetect(
    WindowTypeDetectData *winTypeDetector, 
    float const * inPut, 
    const short frameLength, 
    const short initFrame
) 
{
    float filteredData[FRAME_LEN];
    int16_t windowType = ONLY_LONG_WINDOW;

    // input raw data process: high pass
    SetZero(filteredData, frameLength);
    HpFilter(winTypeDetector->hpHistory, inPut, frameLength, filteredData);

    // calculate block Energies
    CalculateBlockEnergies(filteredData, winTypeDetector, frameLength);

    // calculate window type
    windowType = CalculateWindowType(winTypeDetector, initFrame);

    return windowType;
}
