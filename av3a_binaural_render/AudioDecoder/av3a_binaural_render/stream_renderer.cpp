
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

#include "stream_renderer.h"
#include "avs3_audio.h"
#include "avs3_audio_types.h"
#include "speaker_definition.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <memory>
#include <cassert>
#include <string>
#include <cstring>

#ifndef M_PI
#define M_PI 3.141592653
#endif

static inline std::vector<float> toYggCart(std::vector<float> &polar) {
    float x = sinf(-M_PI * polar[0] / 180.0) * cos(M_PI * polar[1] / 180.0) * polar[2];
    float y = cosf(-M_PI * polar[0] / 180.0) * cos(M_PI * polar[1] / 180.0) * polar[2];
    float z = sinf(M_PI * polar[1] / 180.0) * polar[2];
    return {-x, z, y};
}

inline int OrderFromAcnSequence(int channel) {
    return static_cast<int>(std::floor(sqrt(static_cast<float>(channel))));
}

inline float getGain(float gain, int gainUnit) {
    if (gainUnit == 0) {
        return gain;
    } else {
        return powf(10, gain / 20);
    }
}

inline int DegreeFromAcnSequence(int channel) {
    int order = OrderFromAcnSequence(channel);
    return channel - order * order - order;
}

struct HOAChannel {
    int order;
    int degree;
    int norm;
    bool mute;
    float gain;
    int hasGain;
};

struct ObjChannel {
    float absoluteDistance;
    int id;
    bool mute;
    int gainUnit;
};

struct BedChannel {
    int id;
    bool mute;
};

class AudioContext {

private:

    audio_context *ctx_{nullptr};
    std::map<int, BedChannel> audio_bed_;
    std::map<int, ObjChannel> audio_obj_;
    std::map<int, HOAChannel> audio_hoa_;
    std::vector<float *> planarBuffer_{};
    std::vector<std::vector<float>> planarBufferRef_{};
    bool bypass{false};

public:

    int CreateRenderer(Avs3MetaData *metadata, int sampleRate, int blockSize);

    int PutPlanarAudioBuffer(const float **buffer, int frameNum, int channelNum);

    int PutInterleavedAudioBuffer(const float *buffer, int frameNum, int channelNum);

    int GetBinauralPlanarAudioBuffer(float **buffer, int frameNum);

    int GetBinauralInterleavedAudioBuffer(float *buffer, int frameNum);

    int UpdateMetadata(Avs3MetaData *metadata);

    int SetListenerPosition(float *position, float *front, float *up);

    int DestroyRenderer();
};

int StreamRenderer::CreateRenderer(Avs3MetaData *metadata, int sampleRate, int blockSize) {
    auto ptr = new AudioContext();
    audioContext = ptr;
    return ptr->CreateRenderer(metadata, sampleRate, blockSize);
}

int StreamRenderer::UpdateMetadata(Avs3MetaData *metadata) {
    if (!audioContext) {
        return -1;
    }
    return ((AudioContext *) (audioContext))->UpdateMetadata(metadata);
}

int StreamRenderer::PutPlanarAudioBuffer(const float **buffer, int frameNum, int channelNum) {
    if (!audioContext) {
        return -1;
    }
    return ((AudioContext *) (audioContext))->PutPlanarAudioBuffer(buffer, frameNum, channelNum);
}

int StreamRenderer::PutInterleavedAudioBuffer(const float *buffer, int frameNum, int channelNum) {
    if (!audioContext) {
        return -1;
    }
    return ((AudioContext *) (audioContext))->PutInterleavedAudioBuffer(buffer, frameNum, channelNum);
}

int StreamRenderer::GetBinauralPlanarAudioBuffer(float **buffer, int frameNum) {
    if (!audioContext) {
        return -1;
    }
    return ((AudioContext *) (audioContext))->GetBinauralPlanarAudioBuffer(buffer, frameNum);
}

int StreamRenderer::GetBinauralInterleavedAudioBuffer(float *buffer, int frameNum) {
    if (!audioContext) {
        return -1;
    }
    return ((AudioContext *) (audioContext))->GetBinauralInterleavedAudioBuffer(buffer, frameNum);
}

int StreamRenderer::SetListenerPosition(float *position, float *front, float *up) {
    if (!audioContext) {
        return -1;
    }
    return ((AudioContext *) (audioContext))->SetListenerPosition(position, front, up);
}

int StreamRenderer::DestroyRenderer() {
    if (!audioContext) {
        return -1;
    }
    ((AudioContext *) (audioContext))->DestroyRenderer();
    delete (AudioContext *) audioContext;
    return 0;
}


int AudioContext::CreateRenderer(Avs3MetaData *metadata, int sampleRate, int blockSize) {
    if (!metadata) {
        std::cerr << "metadata id invalid" << std::endl;
        return -1;
    }
    if (audio_create_context(&ctx_, AMBISONIC_SEVENTH_ORDER, blockSize, sampleRate) != SUCCESS) {
        std::cerr << "create context fail" << std::endl;
        return -1;
    }

    if (audio_initialize_context(ctx_) != SUCCESS) {
        std::cerr << "init context fail" << std::endl;
        audio_destroy(ctx_);
        ctx_ = nullptr;
        return -1;
    }

    if (!metadata->hasStaticMeta) {
        std::cerr << "has no staticMetadata" << std::endl;
        return -1;
    }
    auto &staticMeta = metadata->avs3MetaDataStatic.avs3BasicL1;
    auto &audioProgramme = staticMeta.audioProgrammeMeta;
    auto &audioContents = staticMeta.audioContentData;
    auto &audioObjects = staticMeta.audioObjectData;
    auto &audioPacks = staticMeta.audioPackFormatData;
    auto &audioChannels = staticMeta.audioChannelFormatData;

    printf("parse AudioProgramme \n");

    AVS3::SpeakerLayout layout = AVS3::SpeakerLayout::Layout_Unknown;


    for (int i = 0; i < audioProgramme.numContents; ++i) {
        for (int ii = 0; ii < staticMeta.numOfContents; ++ii) {
            auto &audioContent = audioContents[ii];
            if (audioContent.numObjects <= 0 ||
                audioContent.contentIdx != audioProgramme.refContentIdx[i]) {
                continue;
            }

            printf("parse AudioContent %d \n", audioContent.contentIdx);

            for (int j = 0; j < audioContent.numObjects; ++j) {
                for (int jj = 0; jj < staticMeta.numOfObjects; ++jj) {
                    auto &audioObject = audioObjects[jj];
                    if (audioObject.numPacks <= 0 ||
                        audioObject.objectIdx != audioContent.refObjectIdx[j]) {
                        continue;
                    }

                    printf("parse AudioObject %d \n", audioObject.objectIdx);

                    for (int k = 0; k < audioObject.numPacks; ++k) {
                        for (int kk = 0; kk < staticMeta.numOfPacks; ++kk) {
                            auto &audioPack = audioPacks[kk];
                            if (audioPack.numChannels <= 0 ||
                                audioPack.packFormatIdx != audioObject.refPackFormatIdx[k]) {
                                continue;
                            }
                            printf("parse AudioPackFormat %d \n", audioPack.packFormatIdx);

                            std::vector<AVS3::Speaker> speaker;
                            if (audioPack.typeLabel == 1) {
                                auto packId = AVS3::getAudioPackID(1, audioPack.packFormatID, true);
                                for (const auto &item: AVS3::SPEAKER_Layout_2094) {
                                    if (item.second.packFormatId == packId) {
                                        speaker = item.second.speakers;
                                        layout = item.first;
                                        break;
                                    }
                                }
                            }

                            int ch_2094_index = 0;
                            for (int l = 0; l < audioPack.numChannels; ++l) {
                                if (!speaker.empty()) {
                                    assert(speaker.size() == audioPack.numChannels);
                                }
                                for (int ll = 0; ll < staticMeta.numOfChannels; ++ll) {
                                    auto &audioChannel = audioChannels[ll];
                                    if (audioChannel.channelFormatIdx != audioPack.refChannelIdx[l]) {
                                        continue;
                                    }

/******************************************************每个实际音轨对应一个MetadataSource结构**********************************************/

                                    int track = audioChannel.channelFormatIdx + 1;//ADM的音轨索引从1开始
                                    printf("parse AudioChannelFormat %d \n", audioChannel.channelFormatIdx);

                                    if (audioPack.typeLabel == 1) {
                                        float azimuth = 0;
                                        float elevation = 0;
                                        float distance = 1;
                                        if (speaker.empty()) {
                                            azimuth = audioChannel.directSpeakersPositionData.azimuth;
                                            elevation = audioChannel.directSpeakersPositionData.elevation;
                                            distance = audioChannel.directSpeakersPositionData.distance;
                                        } else {
                                            if (ch_2094_index < speaker.size()) {
                                                azimuth = speaker[ch_2094_index].azimuth;
                                                elevation = speaker[ch_2094_index].elevation;
                                                distance = speaker[ch_2094_index].distance;
                                            }
                                            ch_2094_index++;
                                        }
                                        if (audioPack.absoluteDistance > 0) {
                                            distance *= audioPack.absoluteDistance;
                                        }
                                        std::vector<float> polar = {azimuth, elevation, distance};
                                        auto xyz = toYggCart(polar);
                                        int id;
                                        audio_add_source(ctx_, SOURCE_SPATIALIZE, xyz.data(), &id);
                                        if (audioChannel.hasChannelGain) {
                                            audio_set_source_gain(ctx_, id, getGain(audioChannel.channelGain, audioChannel.gainUnit));
                                        } else if (audioObject.hasGain) {
                                            audio_set_source_gain(ctx_, id, getGain(audioObject.gain, audioObject.gainUnit));
                                        }
                                        BedChannel bed{};
                                        bed.id = id;
                                        bed.mute = audioObject.hasMute;
                                        audio_bed_[track] = bed;
                                    } else if (audioPack.typeLabel == 3) {
                                        int id;
                                        std::vector<float> polar = {0, 0, 1};
                                        auto xyz = toYggCart(polar);
                                        audio_add_source(ctx_, SOURCE_SPATIALIZE, xyz.data(), &id);
                                        ObjChannel obj{};
                                        if (audioChannel.hasChannelGain) {
                                            audio_set_source_gain(ctx_, id, getGain(audioChannel.channelGain, audioChannel.gainUnit));
                                            obj.gainUnit = audioChannel.gainUnit;
                                        } else if (audioObject.hasGain) {
                                            audio_set_source_gain(ctx_, id, getGain(audioObject.gain, audioObject.gainUnit));
                                            obj.gainUnit = audioObject.gainUnit;
                                        } else {
                                            obj.gainUnit = 0;
                                        }
                                        obj.id = id;
                                        obj.mute = audioObject.hasMute;
                                        obj.absoluteDistance = audioPack.absoluteDistance;
                                        audio_obj_[track] = obj;
                                    } else if (audioPack.typeLabel == 4) {
                                        HOAChannel hoaChannel{};
                                        hoaChannel.order = OrderFromAcnSequence(l);
                                        hoaChannel.degree = DegreeFromAcnSequence(l);
                                        hoaChannel.norm = audioPack.normalization;
                                        if (audioChannel.hasChannelGain) {
                                            hoaChannel.gain = getGain(audioChannel.channelGain, audioChannel.gainUnit);
                                        } else if (audioObject.hasGain) {
                                            hoaChannel.gain = getGain(audioObject.gain, audioObject.gainUnit);
                                        }
                                        hoaChannel.mute = audioObject.hasMute;
                                        hoaChannel.hasGain = audioChannel.hasChannelGain;
                                        audio_hoa_[track] = hoaChannel;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (audio_bed_.empty() && audio_obj_.empty() && audio_hoa_.empty()) {
        std::cerr << "have not valid source" << std::endl;
        return -1;
    }

    if (audio_bed_.size() == 2 && audio_obj_.empty() && audio_hoa_.empty() && layout == AVS3::SpeakerLayout::Layout_Stereo) {
        bypass = true;
    }

    audio_commit_scene(ctx_);

    return 0;
}

int AudioContext::PutInterleavedAudioBuffer(const float *buffer, int frameNum, int channelNum) {
    if (planarBuffer_.empty()) {
        planarBuffer_.resize(channelNum);
        planarBufferRef_.resize(channelNum);
        for (int i = 0; i < channelNum; ++i) {
            planarBufferRef_[i].resize(frameNum);
            planarBuffer_[i] = planarBufferRef_[i].data();
        }
    }
    for (int j = 0; j < frameNum; ++j) {
        for (int i = 0; i < channelNum; ++i) {
            planarBuffer_[i][j] = buffer[j * channelNum + i];
        }
    }
    return PutPlanarAudioBuffer((const float **) planarBuffer_.data(), frameNum, channelNum);
}

int AudioContext::PutPlanarAudioBuffer(const float **buffer, int frameNum, int channelNum) {
    if (!ctx_) {
        std::cout << "context is not created" << std::endl;
        return -1;
    }
    for (int i = 0; i < channelNum; ++i) {
        int track = i + 1;
        if (audio_bed_.find(track) != audio_bed_.end()) {
            auto bed = audio_bed_[track];
            if (!bed.mute && !bypass) {
                audio_submit_source_buffer(ctx_, bed.id, buffer[i], frameNum);
            } else if (bed.mute) {
                memset((void *)buffer[i], 0, frameNum * sizeof(float));
            }
        }else
        if (audio_obj_.find(track) != audio_obj_.end()) {
            auto obj = audio_obj_[track];
            int id = obj.id;
            if (!obj.mute) {
                audio_submit_source_buffer(ctx_, id, buffer[i], frameNum);
            }
        } else if (audio_hoa_.find(track) != audio_hoa_.end()) {
            auto hoaChannel = audio_hoa_[track];
            if (!hoaChannel.mute) {
                audio_submit_ambisonic_channel_buffer(ctx_, buffer[i], hoaChannel.order,
                                                      hoaChannel.degree,
                                                      hoaChannel.norm == 0 ? SN3D : N3D,
                                                      hoaChannel.hasGain ? hoaChannel.gain : 1);
            }
        } else {
            std::cerr << "not supported track: " << track << std::endl;
        }
    }
    return 0;
}

int AudioContext::GetBinauralInterleavedAudioBuffer(float *buffer, int frameNum) {
    if (!ctx_) {
        std::cout << "context is not created" << std::endl;
        return -1;
    }
    if (bypass) {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < frameNum; ++j) {
                buffer[j * 2 + i] = planarBuffer_[i][j];
            }
        }
    } else {
        audio_get_interleaved_binaural_buffer(ctx_, buffer, frameNum);
    }
    return 0;
}

int AudioContext::GetBinauralPlanarAudioBuffer(float **buffer, int frameNum) {
    if (!ctx_) {
        std::cout << "context is not created" << std::endl;
        return -1;
    }
    if (bypass) {
        for (int i = 0; i < 2; ++i) {
            memcpy(buffer[i], planarBuffer_[i], frameNum * sizeof(float));
        }
    } else {
        audio_get_planar_binaural_buffer(ctx_, buffer, frameNum);
    }
    return 0;
}

int AudioContext::SetListenerPosition(float *position, float *front, float *up) {
    if (!ctx_) {
        std::cout << "context is not created" << std::endl;
        return -1;
    }
    audio_set_listener_pose(ctx_, position, front, up);
    return 0;
}

int AudioContext::UpdateMetadata(Avs3MetaData *metadata) {
    if (!ctx_) {
        std::cout << "context is not created" << std::endl;
        return -1;
    }
    if (!metadata) {
        std::cerr << "metadata id invalid" << std::endl;
        return -1;
    }
    if (metadata->hasDynamicMeta) {
        auto &dm = metadata->avs3MetaDataDynamic;
        std::vector<float> position;
        std::vector<int> track;
        for (int i = 0; i < dm.numDmChans; ++i) {
            int trackIndex = (int) audio_bed_.size() + 1 + i;
            if (audio_obj_.find(trackIndex) == audio_obj_.end()) {
                std::cerr << "not find object track" << std::endl;
                return -1;
            }
            auto obj = audio_obj_[trackIndex];
            int id = obj.id;
            if (dm.avs3DmL1MetaData[i].cartesian == 0) {
                float azimuth = dm.avs3DmL1MetaData[i].objAzimuth;
                float elevation = dm.avs3DmL1MetaData[i].objElevation;
                float distance = dm.avs3DmL1MetaData[i].objDistance;
                if (obj.absoluteDistance > 0) {
                    distance *= obj.absoluteDistance;
                }
                std::vector<float> polar = {azimuth, elevation, distance};
                position = toYggCart(polar);
            } else {
                float x = -dm.avs3DmL1MetaData[i].obj_x;
                float y = dm.avs3DmL1MetaData[i].obj_y;
                float z = dm.avs3DmL1MetaData[i].obj_z;
                if (obj.absoluteDistance > 0) {
                    x *= obj.absoluteDistance;
                    y *= obj.absoluteDistance;
                    z *= obj.absoluteDistance;
                }
                position = {x, y, z};
            }

            if (dm.avs3DmL1MetaData[i].hasObjGain) {
                audio_set_source_gain(ctx_, id, getGain(dm.avs3DmL1MetaData[i].gain, obj.gainUnit));
            }
            audio_set_source_position(ctx_, id, position.data());
        }
    }
    return 0;
}

int AudioContext::DestroyRenderer() {
    if (!ctx_) {
        std::cout << "context is not created" << std::endl;
        return -1;
    }
    audio_destroy(ctx_);
    ctx_ = nullptr;
	
	return 0;
}
