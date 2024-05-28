/* ==========================================================================
  Copyright 2023 HUAWEI TECHNOLOGIES CO., LTD.
  Licensed under the Code Sharing Policy of the UHD World Association (the
  "Policy");
  http://www.theuwa.com/UWA_Code_Sharing_Policy.pdf.
  you may not use this file except in compliance with the Policy.
  Unless agreed to in writing, software distributed under the Policy is
  distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OF ANY KIND, either express or implied.
  See the Policy for the specific language governing permissions and
  limitations under the Policy.
========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "avs3_cnst_com.h"
#include "avs3_rom_com.h"
#include "avs3_prot_com.h"
#include "avs3_prot_dec.h"


// void GetAvs3DecoderCommandLine(AVS3DecoderHandle hAvs3Dec, const int argc, char *argv[], FILE **fBitstream, FILE **fOutput)
// {
//     short i = 1;

// #ifndef BS_HEADER_COMPAT
//     int tmp;

//     while (i < argc - 3)
//     {
//         if (strcmp(toUpper(argv[i]), "-MONO") == 0)
//         {
// #ifdef MONO_INTEGRATE
//             hAvs3Dec->avs3CodecFormat = AVS3_MONO_FORMAT;
// #else
//             hAvs3Dec->avs3CodecFormat = AVS3_STEREO_FORMAT;
// #endif
//             hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
//             hAvs3Dec->numChansOutput = 1;

//             i++;
//         }
//         else if (strcmp(toUpper(argv[i]), "-STEREO") == 0)
//         {
//             hAvs3Dec->avs3CodecFormat = AVS3_STEREO_FORMAT;
//             hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
//             hAvs3Dec->numChansOutput = STEREO_CHANNELS;

//             i++;
//         }
//         else if (strcmp(toUpper(argv[i]), "-MC") == 0)
//         {
// #ifdef MC_ENABLE
//             i++;
//             hAvs3Dec->avs3CodecFormat = AVS3_MC_FORMAT;
//             hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
//             if (i < argc - 3)
//             {
//                 if (sscanf(argv[i], "%d", &tmp) > 0)
//                 {
//                     i++;
//                 }
//             }
//             hAvs3Dec->numChansOutput = tmp;
// #endif
//         }
//         else if (strcmp(toUpper(argv[i]), "-HOA") == 0)
//         {
//             i++;
//             hAvs3Dec->avs3CodecFormat = AVS3_HOA_FORMAT;
//             hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;

//             if (i < argc - 3)
//             {
//                 if (sscanf(argv[i], "%d", &tmp) > 0)
//                 {
//                     i++;
//                 }
//             }

//             switch (tmp)
//             {
//             case 0:
//                 hAvs3Dec->numChansOutput = 1;
//                 break;
//             case 1:
//                 hAvs3Dec->numChansOutput = 4;
//                 break;
//             case 2:
//                 hAvs3Dec->numChansOutput = 9;
//                 break;
//             case 3:
//                 hAvs3Dec->numChansOutput = 16;
//                 break;
//             case 4:
//                 hAvs3Dec->numChansOutput = 25;
//                 break;
//             default:
//                 break;
//             }
//         }
//         else
//         {
//             assert("!Not support codec format in AVS3!\n");
//         }
//     }

//     /* Output sampling rate */
//     if (i < argc - 2)
//     {
//         hAvs3Dec->outputFs = (long)(atoi(argv[i]) * 1000);

//         hAvs3Dec->frameLength = GetFrameLength(hAvs3Dec->outputFs);

//         i++;
//     }
//     else
//     {
//         fprintf(stderr, "Error: Sampling rate is not supported in AVS3!\n");
//         exit(-1);
//     }
// #endif

//     /* Bitstream */
//     if (i < argc - 1)
//     {
//         if ((*fBitstream = fopen(argv[i], "rb")) == NULL)
//         {
//             fprintf(stderr, "Error: Bitstream file %s can not be opened!\n", argv[i]);
//             exit(-1);
//         }

// #ifdef BS_HEADER_COMPAT
//         /* Read bitstream file header info */
//         Avs3ParseBsFrameHeader(hAvs3Dec, *fBitstream, 1, NULL);
// #endif

//         i++;
//     }

//     /* Output file */
//     if (i < argc)
//     {
//         const char* strSuffix = ".wav";
//         const char* startPtr = strrchr(argv[i], '.');

//         if (strncmp(startPtr, strSuffix, strlen(strSuffix)) == 0)
//         {
//             *fOutput = WriteWavHeader(argv[i], hAvs3Dec->numChansOutput, hAvs3Dec->outputFs);
//         }
//         else
//         {
//             if ((*fOutput = fopen(argv[i], "wb+")) == NULL)
//             {
//                 fprintf(stderr, "Error: Output file %s can not be opened!\n", argv[i]);
//                 exit(-1);
//             }
//         }
//     }

//     return;
// }

void WriteSynthData(const short* data, FILE* file, const short nChans, const short frameLength)
{
    if (file == NULL)
    {
        fprintf(stderr, "Output file open error!\n");

        return;
    }

    fwrite(data, sizeof(short), nChans*frameLength, file);
}

void SynthWavHeader(FILE* file)
{
    AVS3_WAVE_HEADER_DATA wavHeader;

    int32_t totalFileSize = 0L;
    int32_t riffSize = 0L;
    int32_t dataSize = 0L;

    if (file == NULL)
    {
        fprintf(stderr, "Output file open error!\n");

        return;
    }

    /* get total file size */
    totalFileSize = ftell(file);

    /* get riff size */
    riffSize = totalFileSize - (sizeof(wavHeader.chunkID) + sizeof(wavHeader.riffSize));

    /* get size */
    dataSize = totalFileSize - sizeof(wavHeader);

    /* seek to riff size field */
    fseek(file, sizeof(wavHeader.chunkID), SEEK_SET);

    /* set riff(chunkID) size */
    fwrite(&riffSize, sizeof(riffSize), 1, file);

    /* seek to data field */
    fseek(file, sizeof(AVS3_WAVE_HEADER_DATA) - sizeof(wavHeader.dataSize), SEEK_SET);

    /* set dataSize field */
    fwrite(&dataSize, sizeof(dataSize), 1, file);

    return;
}