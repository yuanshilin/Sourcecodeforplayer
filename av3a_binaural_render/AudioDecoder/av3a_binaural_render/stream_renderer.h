
/* ==================================================================================================

The copyright in this software is being made available under the License included below.

Copyright (c) 2022 Beijing Zitiao Network Technology Co., Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions
  and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions
  and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the ITU/ISO/IEC nor the names of its contributors may be used to endorse or promote
  products derived from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE
IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

========================================================================================================*/

#ifndef AVS3_RENDER_STREAM_RENDERER_H
#define AVS3_RENDER_STREAM_RENDERER_H

#include "avs3_stat_meta.h"
#include "stdio.h"

class StreamRenderer {

public:

    void *audioContext{nullptr};

public:

    int CreateRenderer(Avs3MetaData *metadata, int sampleRate, int blockSize = 1024);

    int PutPlanarAudioBuffer(const float **buffer, int frameNum, int channelNum);

    int PutInterleavedAudioBuffer(const float *buffer, int frameNum, int channelNum);

    int GetBinauralPlanarAudioBuffer(float **buffer, int frameNum);

    int GetBinauralInterleavedAudioBuffer(float *buffer, int frameNum);

    int UpdateMetadata(Avs3MetaData *metadata);

    int SetListenerPosition(float *position, float *front, float *up);

    int DestroyRenderer();

	FILE* pLog{ nullptr };
};


#endif //AVS3_RENDER_STREAM_RENDERER_H
