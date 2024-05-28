
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "avs3_options.h"
#include "avs3_stat_com.h"


/*
Init range coder config structure
Note: number of cdfs should be configured differently for VAE and Hyper
I/O params:
    FILE *fModel                                (i)   model file
    RangeCoderConfigHandle rangeCoderConfig     (i/o) range coder config handle
    int16_t numCdfs                             (i/o) number of cdfs
*/
int16_t InitRangeCoderConfig(
    modul_structure *fModel,
    RangeCoderConfigHandle rangeCoderConfig,
    int16_t numCdfs
)
{
    uint16_t cdfLength;
    uint32_t cdfValue;

    // For VAE model, number of cdfs equals number of output channels
    // For Hyper-prior model, number of cdfs equals number of variance scales
    rangeCoderConfig->numCdfs = (uint16_t)numCdfs;

    // get cdf length
    rangeCoderConfig->cdfLength = (uint16_t *)malloc(sizeof(uint16_t) * rangeCoderConfig->numCdfs);
    if (rangeCoderConfig->cdfLength == NULL) {
        fprintf(stderr, "Malloc CDF length error!\n");
        exit(-1);
    }
    for (int16_t i = 0; i < rangeCoderConfig->numCdfs; i++) {
//        fread(&cdfLength, sizeof(uint16_t), 1, fModel);
		memcpy(&cdfLength, fModel->data + fModel->nIndex, sizeof(int16_t));
		fModel->nIndex += sizeof(int16_t);
		rangeCoderConfig->cdfLength[i] = cdfLength;
    }

    // get cdf offset
    rangeCoderConfig->offset = (int16_t *)malloc(sizeof(int16_t) * rangeCoderConfig->numCdfs);
    for (int16_t i = 0; i < rangeCoderConfig->numCdfs; i++) {
//        fread(&rangeCoderConfig->offset[i], sizeof(int16_t), 1, fModel);
		memcpy(&rangeCoderConfig->offset[i], fModel->data + fModel->nIndex, sizeof(int16_t));
		fModel->nIndex += sizeof(int16_t);
	}

    // get cdf table for each distribution
    rangeCoderConfig->quantizedCdf = (uint32_t **)malloc(sizeof(uint32_t *) * rangeCoderConfig->numCdfs);
    for (int16_t i = 0; i < rangeCoderConfig->numCdfs; i++) {
        rangeCoderConfig->quantizedCdf[i] = (uint32_t *)malloc(sizeof(uint32_t) * rangeCoderConfig->cdfLength[i]);
    }
    for (int16_t i = 0; i < rangeCoderConfig->numCdfs; i++){
        for (int16_t j = 0; j < rangeCoderConfig->cdfLength[i]; j++) {
//            fread(&cdfValue, sizeof(uint32_t), 1, fModel);
			memcpy(&cdfValue, fModel->data + fModel->nIndex, sizeof(int32_t));
			fModel->nIndex += sizeof(int32_t);
			rangeCoderConfig->quantizedCdf[i][j] = cdfValue;
        }
    }

    // init precision
    rangeCoderConfig->precision = 16;
    // init overflowSize
    rangeCoderConfig->overflowWidth = 4;
    
    return 0;
}


/*
Free range coder config structure
I/O params:
    RangeCoderConfigHandle rangeCoderConfig         (i) range coder config handle
*/
int16_t DestroyRangeCoderConfig(
    RangeCoderConfigHandle rangeCoderConfig
)
{
    free(rangeCoderConfig->cdfLength);
    rangeCoderConfig->cdfLength = NULL;

    free(rangeCoderConfig->offset);
    rangeCoderConfig->offset = NULL;

    for (int16_t i = 0; i < rangeCoderConfig->numCdfs; i++) {
        free(rangeCoderConfig->quantizedCdf[i]);
        rangeCoderConfig->quantizedCdf[i] = NULL;
    }
    free(rangeCoderConfig->quantizedCdf);
    rangeCoderConfig->quantizedCdf = NULL;

    return 0;
}


/*
Init range encoder state st
I/O params:
    RangeEncoderStateHandle rangeEncoderSt          (i/o) RC encoder st handle
*/
int16_t InitRangeEncoderState(
    RangeEncoderStateHandle rangeEncoderSt
)
{
    rangeEncoderSt->base = 0;
    rangeEncoderSt->sizeMinusOne = UINT32_MAX;
    rangeEncoderSt->delay = 0;

    return 0;
}


/*
RC encode single symbol
I/O params:
    RangeEncoderStateHandle rangeEncoderSt          (i/o) RC encoder st handle
    uint32_t lower                                  (i)   lower limit of current interval
    uint32_t upper                                  (i)   upper limit of current interval
    uint16_t precision                              (i)   CDF precision, range: (0, 16] bit
    unsigned char *sink                             (i/o) output bitstream buffer
    int16_t *sinkPointer                            (i/o) pointer or counter in output buffer
*/
static void RangeEncodeSymbol(
    RangeEncoderStateHandle rangeEncoderSt,
    uint32_t lower,
    uint32_t upper,
    uint16_t precision,
    unsigned char *sink,
    int16_t *sinkPointer
)
{
    // check input, 0 < precision < 16
    assert(precision > 0);
    assert(precision <= 16);

    // check input, 0 <= lower < upper <= 2^precision
    assert(0 <= lower);
    assert(lower < upper);
    assert(upper <= ((uint32_t)1 << precision));

    // get intervel size
    const uint64_t size = (const uint64_t)(rangeEncoderSt->sizeMinusOne) + 1;

    // check size, should be greater than 2^16
    assert((size >> 16) != 0);

    // let u := lower and v := upper
    // new intervel should be: [base + (size * u) / 2^precision, base + (size * v) / 2^precision)
    const uint32_t a = (uint32_t)((size * (uint64_t)(lower)) >> precision);
    const uint32_t b = (uint32_t)(((size * (uint64_t)(upper)) >> precision) - 1);

    // check a and b, should be a <= b
    assert(a <= b);

    // The new interval is [base + a, base + b] = [base + a, base + b + 1)
    rangeEncoderSt->base += a;
    rangeEncoderSt->sizeMinusOne = b - a;

    // check overflow of base
    int16_t baseOverflow = 0;
    if (rangeEncoderSt->base < a) {
        baseOverflow = 1;
    }

    // check if 2^32 < base + size
    // if true, new state is 1, previous state is also 1
    if ((rangeEncoderSt->base + rangeEncoderSt->sizeMinusOne) < rangeEncoderSt->base) {

        // check base + size > 2^32
        assert(((rangeEncoderSt->base - a + size) >> 32) != 0);
        // check lower 16bit of delay is not zero, i.e. delay not empty
        assert((rangeEncoderSt->delay & 0xFFFF) != 0);

        // some conclusion: 
        // base <= 2^32 < base + size, and size <= 2^16
        // then: 1) base >= 0xFFFF0000, base[32:16] == 0xFFFF, 2) (base + size - 1)[32:16] == 0x0000

        // 2^32 - base < size, and base >= 0xFFFF0000
        // then 2^16 - base[16:0] < size
        // let base' and size' be the new base and size after bit shift of 16
        // then 2^32 - base' < size', i.e. 2^32 < base' + size'
        // so RC is still in state 1

        // if the new size is <= 2^16, then base and size should be 
        // left shifted by 16 bits
        if (rangeEncoderSt->sizeMinusOne >> 16 == 0) {

            // check base[32:16] equals 0xFFFF
            assert(rangeEncoderSt->base >> 16 == 0xFFFF);

            // bit shift to get new base and size
            rangeEncoderSt->base <<= 16;
            rangeEncoderSt->sizeMinusOne <<= 16;
            rangeEncoderSt->sizeMinusOne |= 0xFFFF;

            // check delay value
            assert(rangeEncoderSt->delay < ((uint64_t)(1) << 62));

            // store the new shifted 16 bits of zeros (as 2 Bytes) to the higher part of delay
            rangeEncoderSt->delay += 0x20000;
        }

        return;
    }

    // current state is 0, previous state is 1
    if (rangeEncoderSt->delay != 0) {

        // case 2
        // interval moved above 2^32, delay' is the converged value after range refinement
        // write out delay[16:0] and write delay[max:16] bytes of 0x00
        // case 3
        // interval moved below 2^32, delay'-1 is the converged value after range refinement
        // write out delay[16:0] and write delay[max:16] bytes of 0xFF
        if (baseOverflow == 1) {
            // case 2
            // check base, greater than 2^32
            assert(((uint64_t)(rangeEncoderSt->base - a) + a) >> 32 != 0);

            // push back delay value
            sink[(*sinkPointer)++] = (char)(rangeEncoderSt->delay >> 8);
            sink[(*sinkPointer)++] = (char)(rangeEncoderSt->delay >> 0);
            for (int16_t i = 0; i < (rangeEncoderSt->delay >> 16); i++) {
                sink[(*sinkPointer)++] = 0;
            }
        }
        else {
            // case 3
            // check base+size-1, lower than 2^32
            assert((uint64_t)(rangeEncoderSt->base + rangeEncoderSt->sizeMinusOne) >> 32 == 0);

            // push back delay value
            rangeEncoderSt->delay--;
            sink[(*sinkPointer)++] = (char)(rangeEncoderSt->delay >> 8);
            sink[(*sinkPointer)++] = (char)(rangeEncoderSt->delay >> 0);
            for (int16_t i = 0; i < (rangeEncoderSt->delay >> 16); i++) {
                sink[(*sinkPointer)++] = 0xFF;
            }
        }

        // reset to state 0
        rangeEncoderSt->delay = 0;
    }

    // current state is 0
    // if size < 2^16, refine interval
    if (rangeEncoderSt->sizeMinusOne >> 16 == 0) {

        const uint32_t top = rangeEncoderSt->base >> 16;

        rangeEncoderSt->base <<= 16;
        rangeEncoderSt->sizeMinusOne <<= 16;
        rangeEncoderSt->sizeMinusOne |= 0xFFFF;

        if (rangeEncoderSt->base <= rangeEncoderSt->base + rangeEncoderSt->sizeMinusOne) {
            // Still in state 0. Write the top 16 bits
            // push back
            sink[(*sinkPointer)++] = (char)(top >> 8);
            sink[(*sinkPointer)++] = (char)(top);
        }
        else {
            // new state is 1
            assert(top < 0xFFFF);
            rangeEncoderSt->delay = top + 1;        // add one in previous, may reduce by one later
        }
    }

    return;
}


/*
Finalize RC encode, output any number in the current interval
I/O params:
    RangeEncoderStateHandle rangeEncoderSt          (i/o) RC encoder st handle
    unsigned char *sink                             (i/o) output bitstream buffer
    int16_t *sinkPointer                            (i/o) pointer or counter in output buffer
*/
static void RangeEncodeFinalize(
    RangeEncoderStateHandle rangeEncoderSt,
    unsigned char *sink,
    int16_t *sinkPointer
)
{
    // Finalize the encoding procedure by writing out any number in the interval of [base, base+size)
    // Trailing zeros are not written out, the decoder could fill in zeros by default
    if (rangeEncoderSt->delay != 0) {
        // The last state was state 1. Since base < 2^32 < base + size, pick 2^32
        // delay is some kind of description for the difference between base and 2^32
        sink[(*sinkPointer)++] = (char)(rangeEncoderSt->delay >> 8);
        if ((rangeEncoderSt->delay & 0xFF) != 0) {
            sink[(*sinkPointer)++] = (char)(rangeEncoderSt->delay);
        }
    }
    else if (rangeEncoderSt->base != 0) {
        // If base == 0, then pick 0 from [base, base + size) and no zeros are written.
        // Otherwise, pick(base + (2 ^ 16 - base[16:0])), i.e., round up base to the
        // next multiple of 2^16. As 2^16 < size, this value should be in the
        // interval [base, base + size)
        const uint32_t mid = ((rangeEncoderSt->base - 1) >> 16) + 1;
        assert((mid & 0xFFFF) == mid);
        sink[(*sinkPointer)++] = (char)(mid >> 8);
        if ((mid & 0xFF) != 0) {
            sink[(*sinkPointer)++] = (char)(mid >> 0);
        }
    }

    // reset range encoder state
    rangeEncoderSt->base = 0;
    rangeEncoderSt->sizeMinusOne = UINT32_MAX;
    rangeEncoderSt->delay = 0;

    return;
}


/*
RC encode top interface, loop over input data
I/O params:
    RangeCoderConfigHandle rangeCoderConfig         (i)   RC config st, include cdf info
    int32_t *data                                   (i)   input data, size: dataLength
    int16_t dataLength                              (i)   input data length
    int16_t *index                                  (i)   index of cdf for each value in 'data'
    unsigned char *sink                             (i/o) output bitstream buffer
    int16_t *sinkPointer                            (i/o) pointer or counter in output buffer
*/
void RangeEncodeProcess(
    RangeCoderConfigHandle rangeCoderConfig,
    int32_t *data,
    int16_t dataLength,
    int16_t *index,
    unsigned char *sink,
    int16_t *sinkPointer
)
{
    // Init range encoder state struct
    RangeEncoderStateStruct rangeEncoderSt;
    InitRangeEncoderState(&rangeEncoderSt);

    // max value of overflow coding
    const uint32_t maxOverflow = (1 << rangeCoderConfig->overflowWidth) - 1;

    for (int16_t i = 0; i < dataLength; i++) {

        // cdf index for current value
        const int16_t cdfIndex = index[i];
        // max value in RC
        // example: cdfLength = 5 = numSymbol+1, possible value[0, 3]
        const int32_t maxValue = rangeCoderConfig->cdfLength[cdfIndex] - 2;

        // get value for coding
        int32_t value = data[i];
        // map value to [0, maxValue]
        value -= rangeCoderConfig->offset[cdfIndex];

        // For overflow condition
        uint32_t overflow = 0;
        if (value < 0) {
            overflow = -2 * value - 1;
            value = maxValue;
        }
        else if (value >= maxValue) {
            overflow = 2 * (value - maxValue);
            value = maxValue;
        }

        // get slice of cdf for coding
        const uint32_t *cdfSlice = rangeCoderConfig->quantizedCdf[cdfIndex];

        // encode single symbol
        RangeEncodeSymbol(&rangeEncoderSt, cdfSlice[value], cdfSlice[value + 1], rangeCoderConfig->precision, sink, sinkPointer);

        // encode overflow values
        if (value == maxValue) {
            // get number of bit sections, section width is overflowWidth
            int32_t widths = 0;
            while ((overflow >> (widths * rangeCoderConfig->overflowWidth)) != 0) {
                ++widths;
            }
            // encode number of bit sections
            uint32_t val = widths;
            while (val >= maxOverflow) {
                RangeEncodeSymbol(&rangeEncoderSt, maxOverflow, maxOverflow + 1, rangeCoderConfig->overflowWidth, sink, sinkPointer);
                val -= maxOverflow;
            }
            RangeEncodeSymbol(&rangeEncoderSt, val, val + 1, rangeCoderConfig->overflowWidth, sink, sinkPointer);
            // encode value for each bit sections
            for (int16_t j = 0; j < widths; ++j) {
                const uint32_t val = (overflow >> (j * rangeCoderConfig->overflowWidth)) & maxOverflow;
                RangeEncodeSymbol(&rangeEncoderSt, val, val + 1, rangeCoderConfig->overflowWidth, sink, sinkPointer);
            }
        }
    }

    // Finalize RC, output tail values
    RangeEncodeFinalize(&rangeEncoderSt, sink, sinkPointer);

    return;
}


/*
RC encode single symbol, bitrate estimation version
I/O params:
    RangeEncoderStateHandle rangeEncoderSt          (i/o) RC encoder st handle
    uint32_t lower                                  (i)   lower limit of current interval
    uint32_t upper                                  (i)   upper limit of current interval
    uint16_t precision                              (i)   CDF precision, range: (0, 16] bit
    int16_t *sinkPointer                            (i/o) pointer or counter in output buffer
*/
static void RangeEncodeSymbolBrEst(
    RangeEncoderStateHandle rangeEncoderSt,
    uint32_t lower,
    uint32_t upper,
    uint16_t precision,
    int16_t *sinkPointer
)
{
    // check input, 0 < precision < 16
    assert(precision > 0);
    assert(precision <= 16);

    // check input, 0 <= lower < upper <= 2^precision
    assert(0 <= lower);
    assert(lower < upper);
    assert(upper <= ((uint32_t)1 << precision));

    // get intervel size
    const uint64_t size = (const uint64_t)(rangeEncoderSt->sizeMinusOne) + 1;

    // check size, should be greater than 2^16
    assert((size >> 16) != 0);

    // let u := lower and v := upper
    // new intervel should be: [base + (size * u) / 2^precision, base + (size * v) / 2^precision)
    const uint32_t a = (uint32_t)((size * (uint64_t)(lower)) >> precision);
    const uint32_t b = (uint32_t)(((size * (uint64_t)(upper)) >> precision) - 1);

    // check a and b, should be a <= b
    assert(a <= b);

    // The new interval is [base + a, base + b] = [base + a, base + b + 1)
    rangeEncoderSt->base += a;
    rangeEncoderSt->sizeMinusOne = b - a;

    // check overflow of base
    int16_t baseOverflow = 0;
    if (rangeEncoderSt->base < a) {
        baseOverflow = 1;
    }

    // check if 2^32 < base + size
    // if true, new state is 1, previous state is also 1
    if ((rangeEncoderSt->base + rangeEncoderSt->sizeMinusOne) < rangeEncoderSt->base) {

        // check base + size > 2^32
        assert(((rangeEncoderSt->base - a + size) >> 32) != 0);
        // check lower 16bit of delay is not zero, i.e. delay not empty
        assert((rangeEncoderSt->delay & 0xFFFF) != 0);

        // some conclusion: 
        // base <= 2^32 < base + size, and size <= 2^16
        // then: 1) base >= 0xFFFF0000, base[32:16] == 0xFFFF, 2) (base + size - 1)[32:16] == 0x0000

        // 2^32 - base < size, and base >= 0xFFFF0000
        // then 2^16 - base[16:0] < size
        // let base' and size' be the new base and size after bit shift of 16
        // then 2^32 - base' < size', i.e. 2^32 < base' + size'
        // so RC is still in state 1

        // if the new size is <= 2^16, then base and size should be 
        // left shifted by 16 bits
        if (rangeEncoderSt->sizeMinusOne >> 16 == 0) {

            // check base[32:16] equals 0xFFFF
            assert(rangeEncoderSt->base >> 16 == 0xFFFF);

            // bit shift to get new base and size
            rangeEncoderSt->base <<= 16;
            rangeEncoderSt->sizeMinusOne <<= 16;
            rangeEncoderSt->sizeMinusOne |= 0xFFFF;

            // check delay value
            assert(rangeEncoderSt->delay < ((uint64_t)(1) << 62));

            // store the new shifted 16 bits of zeros (as 2 Bytes) to the higher part of delay
            rangeEncoderSt->delay += 0x20000;
        }

        return;
    }

    // current state is 0, previous state is 1
    if (rangeEncoderSt->delay != 0) {

        // case 2
        // interval moved above 2^32, delay' is the converged value after range refinement
        // write out delay[16:0] and write delay[max:16] bytes of 0x00
        // case 3
        // interval moved below 2^32, delay'-1 is the converged value after range refinement
        // write out delay[16:0] and write delay[max:16] bytes of 0xFF
        if (baseOverflow == 1) {
            // case 2
            // check base, greater than 2^32
            assert(((uint64_t)(rangeEncoderSt->base - a) + a) >> 32 != 0);

            // push back delay value
            (*sinkPointer)++;
            (*sinkPointer)++;
            for (int16_t i = 0; i < (rangeEncoderSt->delay >> 16); i++) {
                (*sinkPointer)++;
            }
        }
        else {
            // case 3
            // check base+size-1, lower than 2^32
            assert((uint64_t)(rangeEncoderSt->base + rangeEncoderSt->sizeMinusOne) >> 32 == 0);

            // push back delay value
            rangeEncoderSt->delay--;
            (*sinkPointer)++;
            (*sinkPointer)++;
            for (int16_t i = 0; i < (rangeEncoderSt->delay >> 16); i++) {
                (*sinkPointer)++;
            }
        }

        // reset to state 0
        rangeEncoderSt->delay = 0;
    }

    // current state is 0
    // if size < 2^16, refine interval
    if (rangeEncoderSt->sizeMinusOne >> 16 == 0) {

        const uint32_t top = rangeEncoderSt->base >> 16;

        rangeEncoderSt->base <<= 16;
        rangeEncoderSt->sizeMinusOne <<= 16;
        rangeEncoderSt->sizeMinusOne |= 0xFFFF;

        if (rangeEncoderSt->base <= rangeEncoderSt->base + rangeEncoderSt->sizeMinusOne) {
            // Still in state 0. Write the top 16 bits
            // push back
            (*sinkPointer)++;
            (*sinkPointer)++;
        }
        else {
            // new state is 1
            assert(top < 0xFFFF);
            rangeEncoderSt->delay = top + 1;        // add one in previous, may reduce by one later
        }
    }

    return;
}


/*
Finalize RC encode, output any number in the current interval, bitrate estimation version
I/O params:
    RangeEncoderStateHandle rangeEncoderSt          (i/o) RC encoder st handle
    int16_t *sinkPointer                            (i/o) pointer or counter in output buffer
*/
static void RangeEncodeFinalizeBrEst(
    RangeEncoderStateHandle rangeEncoderSt,
    int16_t *sinkPointer
)
{
    // Finalize the encoding procedure by writing out any number in the interval of [base, base+size)
    // Trailing zeros are not written out, the decoder could fill in zeros by default
    if (rangeEncoderSt->delay != 0) {
        // The last state was state 1. Since base < 2^32 < base + size, pick 2^32
        // delay is some kind of description for the difference between base and 2^32
        (*sinkPointer)++;
        if ((rangeEncoderSt->delay & 0xFF) != 0) {
            (*sinkPointer)++;
        }
    }
    else if (rangeEncoderSt->base != 0) {
        // If base == 0, then pick 0 from [base, base + size) and no zeros are written.
        // Otherwise, pick(base + (2 ^ 16 - base[16:0])), i.e., round up base to the
        // next multiple of 2^16. As 2^16 < size, this value should be in the
        // interval [base, base + size)
        const uint32_t mid = ((rangeEncoderSt->base - 1) >> 16) + 1;
        assert((mid & 0xFFFF) == mid);
        (*sinkPointer)++;
        if ((mid & 0xFF) != 0) {
            (*sinkPointer)++;
        }
    }

    // reset range encoder state
    rangeEncoderSt->base = 0;
    rangeEncoderSt->sizeMinusOne = UINT32_MAX;
    rangeEncoderSt->delay = 0;

    return;
}


/*
RC encode top interface, loop over input data, bitrate estimation version
I/O params:
    RangeCoderConfigHandle rangeCoderConfig         (i)   RC config st, include cdf info
    int32_t *data                                   (i)   input data, size: dataLength
    int16_t dataLength                              (i)   input data length
    int16_t *index                                  (i)   index of cdf for each value in 'data'
    int16_t *sinkPointer                            (i/o) pointer or counter in output buffer
*/
void RangeEncodeProcessBrEst(
    RangeCoderConfigHandle rangeCoderConfig,
    int32_t *data,
    int16_t dataLength,
    int16_t *index,
    int16_t *sinkPointer
)
{
    // Init range encoder state struct
    RangeEncoderStateStruct rangeEncoderSt;
    InitRangeEncoderState(&rangeEncoderSt);

    // max value of overflow coding
    const uint32_t maxOverflow = (1 << rangeCoderConfig->overflowWidth) - 1;

    for (int16_t i = 0; i < dataLength; i++) {

        // cdf index for current value
        const int16_t cdfIndex = index[i];
        // max value in RC
        // example: cdfLength = 5 = numSymbol+1, possible value[0, 3]
        const int32_t maxValue = rangeCoderConfig->cdfLength[cdfIndex] - 2;

        // get value for coding
        int32_t value = data[i];
        // map value to [0, maxValue]
        value -= rangeCoderConfig->offset[cdfIndex];

        // For overflow condition
        uint32_t overflow = 0;
        if (value < 0) {
            overflow = -2 * value - 1;
            value = maxValue;
        }
        else if (value >= maxValue) {
            overflow = 2 * (value - maxValue);
            value = maxValue;
        }

        // get slice of cdf for coding
        const uint32_t *cdfSlice = rangeCoderConfig->quantizedCdf[cdfIndex];

        // encode single symbol
        RangeEncodeSymbolBrEst(&rangeEncoderSt, cdfSlice[value], cdfSlice[value + 1], rangeCoderConfig->precision, sinkPointer);

        // encode overflow values
        if (value == maxValue) {
            // get number of bit sections, section width is overflowWidth
            int32_t widths = 0;
            while ((overflow >> (widths * rangeCoderConfig->overflowWidth)) != 0) {
                ++widths;
            }
            // encode number of bit sections
            uint32_t val = widths;
            while (val >= maxOverflow) {
                RangeEncodeSymbolBrEst(&rangeEncoderSt, maxOverflow, maxOverflow + 1, rangeCoderConfig->overflowWidth, sinkPointer);
                val -= maxOverflow;
            }
            RangeEncodeSymbolBrEst(&rangeEncoderSt, val, val + 1, rangeCoderConfig->overflowWidth, sinkPointer);
            // encode value for each bit sections
            for (int16_t j = 0; j < widths; ++j) {
                const uint32_t val = (overflow >> (j * rangeCoderConfig->overflowWidth)) & maxOverflow;
                RangeEncodeSymbolBrEst(&rangeEncoderSt, val, val + 1, rangeCoderConfig->overflowWidth, sinkPointer);
            }
        }
    }

    // Finalize RC, output tail values
    RangeEncodeFinalizeBrEst(&rangeEncoderSt, sinkPointer);

    return;
}


/*
Init range decoder state st
I/O params:
    RangeDecoderStateHandle rangeDecoderSt          (i/o) RC decoder st handle
*/
int16_t InitRangeDecoderState(
    RangeDecoderStateHandle rangeDecoderSt
)
{
    rangeDecoderSt->base = 0;
    rangeDecoderSt->sizeMinusOne = UINT32_MAX;
    rangeDecoderSt->value = 0;

    return 0;
}


/*
Read 16bit data (2 Bytes) from bitstream
Put it into st->value
I/O params:
    RangeDecoderStateHandle rangeDecoderSt          (i/o) RC decoder st handle
    unsigned char *input                            (i)   input bitstream buffer
    int16_t inputLength                             (i)   input bitstream buffer length
    int16_t *inputPointer                           (i/o) current pointer or counter for input buffer
*/
static void Read16BitData(
    RangeDecoderStateHandle rangeDecoderSt,
    unsigned char *input,
    int16_t inputLength,
    int16_t *inputPointer
)
{
    rangeDecoderSt->value <<= 8;
    if (*inputPointer != inputLength) {
        rangeDecoderSt->value |= input[*inputPointer];
        (*inputPointer)++;
    }

    rangeDecoderSt->value <<= 8;
    if (*inputPointer != inputLength) {
        rangeDecoderSt->value |= input[*inputPointer];
        (*inputPointer)++;
    }

    return;
}


static int32_t CdfSearchLen5(
    const uint64_t size,
    const uint64_t offset,
    const uint32_t *cdf)
{
    int32_t pv;

    if (size * (uint64_t)(cdf[1]) > offset) {
        pv = 1;
    } else if (size * (uint64_t)(cdf[2]) > offset) {
        pv = 2;
    } else if (size * (uint64_t)(cdf[3]) > offset) {
        pv = 3;
    } else {
        pv = 4;
    }

    return pv;
}


static int32_t CdfSearchNormal(
    const uint64_t size,
    const uint64_t offset,
    const uint32_t *cdf,
    uint16_t cdfLength)
{
    int32_t i = 1;
    int32_t end = cdfLength - 1;
    int32_t length = end - i;
    int32_t mid;
    int32_t pv;

    while (length > 64) {
        mid = i + (length >> 1);
        if (size * (uint64_t)(cdf[mid]) <= offset) {
            i = mid;
        } else {
            end = mid;
        }
        length = end - i;
    }
    for (i = i + 3; i < end; i += 4) {
        if (size * (uint64_t)(cdf[i]) > offset) {
            break;
        }
    }
    if (i > end) {
        i = end;
    }

    if (size * (uint64_t)(cdf[i - 3]) > offset) {
        pv = i - 3;
    } else if (size * (uint64_t)(cdf[i - 2]) > offset) {
        pv = i - 2;
    } else if (size * (uint64_t)(cdf[i - 1]) > offset) {
        pv = i - 1;
    } else {
        pv = i;
    }

    return pv;
}


/*
RC decode single symbol
I/O params:
    RangeDecoderStateHandle rangeDecoderSt          (i/o) RC decoder st handle
    uint32_t *cdf                                   (i)   CDF for current data value
    uint16_t cdfLength                              (i)   CDF length
    uint16_t precision                              (i)   CDF precision, range: (0, 16] bit
    unsigned char *input                            (i)   input bitstream buffer
    int16_t inputLength                             (i)   valid size of input bistream buffer
    int16_t *inputPointer                           (i/o) pointer or counter in output buffer
*/
static int32_t RangeDecodeSymbol(
    RangeDecoderStateHandle rangeDecoderSt,
    uint32_t *cdf,
    uint16_t cdfLength,
    uint16_t precision,
    unsigned char *input,
    int16_t inputLength,
    int16_t *inputPointer
)
{
    // first time, read 32bit data
    if (*inputPointer == 0) {
        Read16BitData(rangeDecoderSt, input, inputLength, inputPointer);
        Read16BitData(rangeDecoderSt, input, inputLength, inputPointer);
    }

    // check precision, 0 < precision <= 16
    assert(0 < precision);
    assert(precision <= 16);

    const uint64_t size = (uint64_t)(rangeDecoderSt->sizeMinusOne) + 1;
    const uint64_t offset = (((uint64_t)(rangeDecoderSt->value - rangeDecoderSt->base) + 1) << precision) - 1;

    // After the binary search, `pv` points to the smallest number v that
    // satisfies offset < (size * v) / 2^precision

    // Assumes that cdf[0] == 0. Therefore (size * cdf[0]) / 2^precision is always
    // less than or equal to offset
    int32_t pv = 1;

    // len can be cdf.size() - 2 if there is guarantee that the last element of
    // cdf is 2^precision
    if (cdfLength == 5) {
        pv = CdfSearchLen5(size, offset, cdf);
    } else {
        pv = CdfSearchNormal(size, offset, cdf, cdfLength);
    }

    // If (size * v) / 2^precision <= offset for all v in cdf, then pv points to
    // one after the last element of cdf. That is a decoding error
    assert(pv < cdfLength);

    const uint32_t a = (uint32_t)((size * (uint64_t)(cdf[pv - 1])) >> precision);
    const uint32_t b = (uint32_t)(((size * (uint64_t)(cdf[pv])) >> precision) - 1);
    assert(a <= (offset >> precision));
    assert((offset >> precision) <= b);

    rangeDecoderSt->base += a;
    rangeDecoderSt->sizeMinusOne = b - a;

    if (rangeDecoderSt->sizeMinusOne >> 16 == 0) {
        rangeDecoderSt->base <<= 16;
        rangeDecoderSt->sizeMinusOne <<= 16;
        rangeDecoderSt->sizeMinusOne |= 0xFFFF;

        Read16BitData(rangeDecoderSt, input, inputLength, inputPointer);
    }

    return (pv - 1);
}


/*
RC decode top interface, loop over input data
I/O params:
    RangeCoderConfigHandle rangeCoderConfig         (i)   RC config st, include cdf info
    int32_t *data                                   (o)   decoded data buffer, size: dataLength
    int16_t dataLength                              (i)   decoded data buffer length
    int16_t *index                                  (i)   index of cdf for each value in 'data'
    unsigned char *input                            (i)   input bitstream buffer
    int16_t inputLength                             (i)   valid length of input bitstream buffer
*/
void RangeDecodeProcess(
    RangeCoderConfigHandle rangeCoderConfig,
    int32_t *data,
    int16_t dataLength,
    int16_t *index,
    unsigned char *input,
    int16_t inputLength
)
{
    // Init range decoder state struct
    RangeDecoderStateStruct rangeDecoderSt;
    InitRangeDecoderState(&rangeDecoderSt);

    const uint32_t maxOverflow = (1 << rangeCoderConfig->overflowWidth) - 1;

    const int32_t overflowCdfSize = (1 << rangeCoderConfig->overflowWidth) + 1;
    uint32_t *overflowCdf = NULL;
    overflowCdf = (uint32_t *)malloc(sizeof(uint32_t) * overflowCdfSize);
    for (int32_t i = 0; i < overflowCdfSize; i++) {
        overflowCdf[i] = (uint32_t)i;
    }

    int16_t inputPointer = 0;
    for (int16_t i = 0; i < dataLength; i++) {

        // cdf index for current value
        const int32_t cdfIndex = index[i];
        // max value in RC
        // example: cdfLength = 5 = numSymbol+1, possible value[0, 3]
        const int32_t maxValue = rangeCoderConfig->cdfLength[cdfIndex] - 2;

        // get slice of cdf for decoding
        uint32_t *cdfSlice = rangeCoderConfig->quantizedCdf[cdfIndex];

        // encode single symbol
        int32_t value = RangeDecodeSymbol(&rangeDecoderSt, cdfSlice, rangeCoderConfig->cdfLength[cdfIndex], 
            rangeCoderConfig->precision, input, inputLength, &inputPointer);

        // Decode overflow using variable length code
        if (value == maxValue) {

            int32_t widths = 0;
            uint32_t val;

            // decode number of bit sections, section width is overflowWidth
            do{
                val = RangeDecodeSymbol(&rangeDecoderSt, overflowCdf, overflowCdfSize,
                    rangeCoderConfig->overflowWidth, input, inputLength, &inputPointer);
                widths += val;
            } while (val == maxOverflow);

            // decode value for each bit sections
            uint32_t overflow = 0;
            for (int32_t j = 0; j < widths; ++j) {
                const uint32_t val = RangeDecodeSymbol(&rangeDecoderSt, overflowCdf, overflowCdfSize,
                    rangeCoderConfig->overflowWidth, input, inputLength, &inputPointer);
                // concat each section of overflow
                overflow |= val << (j * rangeCoderConfig->overflowWidth);
            }

            // map positive values back to integer values
            value = overflow >> 1;
            if (overflow & 1) {
                value = -value - 1;
            }
            else {
                value += maxValue;
            }
        }

        // Map values in 0..max_range range back to original integer range
        value += rangeCoderConfig->offset[cdfIndex];
        data[i] = value;
    }

    //reset range decoder state
    rangeDecoderSt.base = 0;
    rangeDecoderSt.sizeMinusOne = UINT32_MAX;
    rangeDecoderSt.value = 0;

    free(overflowCdf);
    overflowCdf = NULL;
}
