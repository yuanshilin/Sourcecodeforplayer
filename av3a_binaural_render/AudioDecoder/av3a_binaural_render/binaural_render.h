/* Copyright 2021 Beijing Zitiao Network Technology Co.,
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef AVS3_RENDER_BINAURAL_RENDER_H
#define AVS3_RENDER_BINAURAL_RENDER_H

#include "avs3_render.h"
#include "avs3_audio.h"
#include "avs3_audio_types.h"
#include <cmath>
#include <unordered_map>
#include <vector>

#ifndef M_PI
#define M_PI 3.141592653
#endif

namespace AVS3 {

class BinauralRender: public AVS3Render<Metadata> {

public:

    explicit BinauralRender(const shared_ptr<Metadata> &metadata, RenderMode mode);

    int putAudioData(float **input, int channelCount, int sampleNum) override;

    int putAudioData(float *input, int channelCount, int sampleNum) override;

    int getAudioData(float **output, int sampleCount) override;

    int getAudioData(float *output, int sampleCount) override;

    int setObjectPosition(float **position, int* trackIndex, int trackNum) override;

    int setListenerPosition(float *position, float *front, float *up) override;

    int destroyRender() override;

    ~BinauralRender() override;

private:

    audio_context *ctx{nullptr};
    std::unordered_map<int, int> channelSources;
    std::unordered_map<int, int> objectSources;
    std::unordered_map<int, std::shared_ptr<HOARenderFrame>> hoa;
    std::vector<std::vector<float>> planarTmpBufferRef;
    std::vector<float *> planarTmpBuffer;
    RenderMode renderMode{File};
    std::shared_ptr<Metadata> metadata{nullptr};

};
}
#endif //AVS3_RENDER_BINAURAL_RENDER_H
