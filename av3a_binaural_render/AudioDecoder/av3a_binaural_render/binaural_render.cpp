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

#include "binaural_render.h"
#include <cassert>
#include <iostream>

constexpr int absoluteDistance = 1;

namespace AVS3 {

BinauralRender::BinauralRender(const shared_ptr<Metadata> &metadata, RenderMode mode) : renderMode(mode), metadata(metadata) {
    int level = metadata->getAmbisonicOrder();
    int framePerBuffer = metadata->framePerBuffer;
    int sampleRate = metadata->sampleRate;
    auto ret = audio_create_context(&ctx, (rendering_mode) AMBISONIC_SEVENTH_ORDER, framePerBuffer, sampleRate);
    if (ret != SUCCESS) {
        std::cerr << "audio context created fail" << std::endl;
    }
    ret = audio_initialize_context(ctx);
    if (ret != SUCCESS) {
        std::cerr << "audio context init fail" << std::endl;
    }
    ret = audio_commit_scene(ctx);
    assert(ret == SUCCESS);
    float position[3] = {.0, .0, .0};
    for (auto &ao: metadata->metaDataObjects) {
        for (auto &as :ao->metaDataSources) {
            int trackIndex = (int) as->trackIndex;
            if (as->type == AVS3::DirectSpeakers) {
                int id;
                auto frame = as->directSpeakerFrame;
                position[0] = frame->x * absoluteDistance;
                position[1] = frame->y * absoluteDistance;
                position[2] = frame->z * absoluteDistance;
                ret = audio_add_source(ctx, SOURCE_SPATIALIZE, position, &id);
                assert(ret == SUCCESS);
                ret = audio_set_source_gain(ctx, id, frame->gain);
                assert(ret == SUCCESS);
                channelSources[trackIndex] = id;
            } else if (as->type == AVS3::Object) {
                int id;
                auto frame = as->objectFrames[0];
                position[0] = frame->x * absoluteDistance;
                position[1] = frame->y * absoluteDistance;
                position[2] = frame->z * absoluteDistance;
                ret = audio_add_source(ctx, SOURCE_SPATIALIZE, position, &id);
                assert(ret == SUCCESS);
                ret = audio_set_source_gain(ctx, id, frame->gain);
                assert(ret == SUCCESS);
                objectSources[trackIndex] = id;
            } else if (as->type == HOA) {
                auto frame = as->hoaFrame;
                auto hoaFrame = std::make_shared<HOARenderFrame>();
                hoaFrame->order = frame->order;
                hoaFrame->degree = frame->degree;
                hoaFrame->gain = frame->gain;
                hoaFrame->normalization = frame->normalization;
                hoa[trackIndex] = hoaFrame;
            } else {
                std::cerr << "ADM type not support" << std::endl;
            }
        }
    }
}

int BinauralRender::putAudioData(float *input, int channelCount, int sampleNum) {
    if (planarTmpBuffer.empty()) {
        planarTmpBuffer.resize(channelCount);
        planarTmpBufferRef.resize(channelCount);
        for (int i = 0; i < channelCount; ++i) {
            planarTmpBufferRef[i].resize(sampleNum);
            planarTmpBuffer[i] = planarTmpBufferRef[i].data();
        }
    }
    for (int j = 0; j < sampleNum; ++j) {
        for (int i = 0; i < channelCount; ++i) {
            planarTmpBuffer[i][j] = input[j * channelCount + i];
        }
    }
    return putAudioData(planarTmpBuffer.data(), channelCount, sampleNum);
}

int BinauralRender::putAudioData(float **input, int channelCount, int sampleNum) {
    if (renderMode == File) {
        float position[3] = {.0, .0, .0};
        for (const auto &obj : objectSources) {
            auto frame = metadata->getNextObjectFrame(obj.first);
            if (frame) {
                int id = obj.second;
                position[0] = frame->x * absoluteDistance;
                position[1] = frame->y * absoluteDistance;
                position[2] = frame->z * absoluteDistance;
                auto ret = audio_set_source_position(ctx, id, position);
                assert(ret == SUCCESS);
                ret = audio_set_source_gain(ctx, id, frame->gain);
                assert(ret == SUCCESS);
            }
        }
    }
    int ret = audio_update_scene(ctx);
    assert(ret == SUCCESS);
    for (int i = 0; i < channelCount; ++i) {
        if (channelSources.find(i) != channelSources.end()) {
            int id = channelSources[i];
            ret = audio_submit_source_buffer(ctx, id, input[i], sampleNum);
            assert(ret == SUCCESS);
        } else if (objectSources.find(i) != objectSources.end()) {
            int id = objectSources[i];
            ret = audio_submit_source_buffer(ctx, id, input[i], sampleNum);
            assert(ret == SUCCESS);
        } else if (hoa.find(i) != hoa.end()) {
            auto &hoaFrame = hoa[i];
            auto norm = hoaFrame->normalization == 0 ? SN3D : N3D;
            ret = audio_submit_ambisonic_channel_buffer(ctx, input[i], hoaFrame->order, hoaFrame->degree,
                                                        norm, hoaFrame->gain);
            assert(ret == SUCCESS);
        } else {
            std::cerr << "audio track not match with metadata" << std::endl;
        }
    }
    return 0;
}

int BinauralRender::getAudioData(float *output, int sampleCount) {
    auto ret = audio_get_interleaved_binaural_buffer(ctx, output, sampleCount);
    return ret == SUCCESS ? 0 : -1;
}

int BinauralRender::getAudioData(float **output, int sampleCount) {
    auto ret = audio_get_planar_binaural_buffer(ctx, output, sampleCount);
    return ret == SUCCESS ? 0 : -1;
}

int BinauralRender::setObjectPosition(float **position, int *trackIndex, int trackNum) {
    if (renderMode == File) {
        return -1;
    }
    for (int i = 0; i < trackNum; ++i) {
        int index = trackIndex[i];
        if (objectSources.find(index) == objectSources.end()) {
            std::cerr << "not found track: " << index << "in metadata" << std::endl;
            return -1;
        }
        int id = objectSources[index];
        auto ret = audio_set_source_position(ctx, id, position[i]);
        assert(ret == SUCCESS);
    }
    return 0;
}

int BinauralRender::setListenerPosition(float *position, float *front, float *up) {
    auto ret = audio_set_listener_pose(ctx, position, front, up);
    return ret == SUCCESS ? 0 : -1;
}

int BinauralRender::destroyRender() {
    if (ctx) {
        auto ret = audio_destroy(ctx);
        ctx = nullptr;
        return ret == SUCCESS ? 0 : -1;
    }
    return -1;
}

BinauralRender::~BinauralRender() = default;;
}
