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

#include "metadata.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string.h>

#ifndef M_PI
#define M_PI 3.141592653
#endif

namespace AVS3 {

Metadata::Metadata(shared_ptr<AVS3::ADM> &adm, int sampleRate, int framePerBuffer)
        : sampleRate(sampleRate), framePerBuffer(framePerBuffer) {
    float frame_duration = 1000 * (float) framePerBuffer / (float) sampleRate;
    std::cout << "convert metadata..." << std::endl;
    assert(!adm->getAudioObject().empty());
    for (auto &ao : adm->getAudioObject()) {
        auto metadataObject = convertObject(const_cast<shared_ptr<AudioObject> &>(ao), frame_duration);
        metaDataObjects.push_back(metadataObject);
    }
    for (auto &apm : adm->getAudioProgram()) {
        auto metaDataProgramme = std::make_shared<MetadataProgram>();
        metaDataProgramme->loudnessMetaData = apm->loudnessMetaData;
        metaDataProgramme->name = apm->name;
        metaDataProgramme->id = apm->id;
        metaDataProgram.push_back(metaDataProgramme);
        for (auto &at : apm->audioContent) {
            auto metaDataContent = std::make_shared<MetadataContent>();
            metaDataContent->loudnessMetaData = at->loudnessMetadata;
            metaDataContent->id = at->id;
            metaDataContent->name = at->name;
            metaDataProgramme->metaDataContents.push_back(metaDataContent);
            for (auto &ao : at->audioObjects) {
                metaDataContent->metaDataObjectsRefID.push_back(ao->id);
            }
        }
    }
}

std::shared_ptr<MetadataObject> Metadata::convertObject(std::shared_ptr<AVS3::AudioObject> &ao, float frame_duration) const {
    auto metaDataObject = std::make_shared<MetadataObject>();
    metaDataObject->start = ao->start;
    metaDataObject->duration = ao->duration;
    int i = 0;
    for (auto &atu : ao->audioTrackUIDs) {
        auto metaDataSource = std::make_shared<MetadataSource>();
        if (atu->sampleRate != sampleRate) {
            std::cerr << "adm sampleRate not equal audio sampleRate" << std::endl;
        }
        metaDataSource->sampleRate = atu->sampleRate;
        metaDataSource->bitDepth = atu->bitDepth;
        metaDataSource->audioTrackUID = atu->id;
        metaDataObject->metaDataSources.push_back(metaDataSource);

        std::shared_ptr<AudioChannelFormat> acf{nullptr};
        if (atu->audioTrackFormat.empty()) {
            if (ao->audioPackFormats.empty()) {
                metaDataSource->type = AVS3::DirectSpeakers;
                auto item = std::make_shared<DirectSpeakerFrame>();
                item->x = 0;
                item->y = 0;
                item->z = 1;
                item->gain = 1;
                metaDataSource->directSpeakerFrame = item;
                metaDataSource->type = DirectSpeakers;
                continue;
            } else {
                if (!atu->audioChannelFormat.empty()) {
                    acf = atu->audioChannelFormat[0];
                } else if (!ao->audioPackFormats.empty()) {
                    acf = ao->audioPackFormats[0]->audioChannelFormats[i];
                } else {
                    std::cerr << "no AudioChannelFormat";
                    exit(-1);
                }
            }
        } else {
            assert(atu->audioTrackFormat.size() == 1);
            auto atf = atu->audioTrackFormat[0];
            assert(atf->audioStreamFormat);
            auto asf = atf->audioStreamFormat;

            assert(asf->audioTrackFormat.size() == 1);
            if (asf->audioTrackFormat[0]->id != atf->id) {
                std::cout << "audioStreamFormat verification fail" << endl;
                exit(-2);
            }

            assert(asf->audioChannelFormat.size() == 1);
            acf = asf->audioChannelFormat[0];
        }
        i++;

        metaDataSource->type = acf->type;
        if (acf->type == Object) {
            metaDataSource->objectFrames = parseObjectFrame(acf->audioBlockFormat, frame_duration);
        } else if (acf->type == HOA) {
            for (auto &ab: acf->audioBlockFormat) {
                auto item = std::make_shared<HOARenderFrame>();
                item->order = ab->order;
                item->normalization = std::stoi(ab->normalization);
                item->degree = ab->degree;
                item->gain = ab->gain;
                metaDataSource->hoaFrame = item;
            }
        } else if (acf->type == DirectSpeakers) {
            for (auto &ab: acf->audioBlockFormat) {
                auto item = std::make_shared<DirectSpeakerFrame>();
                if (ab->cartesian == 0) {
                    auto azimuth_ = (float) (ab->azimuth / 180.0f * M_PI);
                    auto elevation_ = (float) (ab->elevation / 180.0f * M_PI);
                    item->x = ab->distance * std::cos(elevation_) * std::sin(azimuth_);
                    item->y = ab->distance * std::sin(elevation_);
                    item->z = ab->distance * std::cos(elevation_) * std::cos(azimuth_);
                } else {
                    item->x = -ab->x;
                    item->y = ab->z;
                    item->z = ab->y;
                }
                item->gain = ab->gain;
                metaDataSource->directSpeakerFrame = item;
            }
        } else if (acf->type == Matrix) {
            for (auto &ab: acf->audioBlockFormat) {
                auto item = std::make_shared<MatrixFrame>();
                item->matrix.push_back(1);
                metaDataSource->matrixFrame = item;
            }
            auto id = acf->id.substr(acf->id.size() - 1, acf->id.size());
        } else if (acf->type == Binaural) {
            auto item = std::make_shared<BinauralFrame>();
            if (acf->name == "LeftEar") {
                item->channel_index = 0;
            } else if (acf->name == "RightEar") {
                item->channel_index = 1;
            } else {
                exit(-3);
            }
            metaDataSource->binauralFrame = item;
        } else {
            assert(false);
        }
    }

    //for verify
    if (!ao->audioPackFormats.empty()) {
        assert(ao->audioPackFormatIDRef.size() == 1);
        for (const auto &atu : ao->audioTrackUIDs) {
            if (atu->audioTrackFormat.empty()) {
                continue;
            }
            auto acf = atu->audioTrackFormat[0]->audioStreamFormat->audioChannelFormat[0];
            auto acf_id = acf->id;
            assert(acf->type == ao->audioPackFormats[0]->type);
            int verified = 0;
            for (const auto &acf_p : ao->audioPackFormats[0]->audioChannelFormats) {
                if (acf_id == acf_p->id) {
                    verified = 1;
                    break;
                }
            }
            assert(verified == 1);
        }
    }
    return metaDataObject;
}

std::vector<shared_ptr<ObjectFrame>> Metadata::parseObjectFrame(list<shared_ptr<AudioBlockFormat>> &block, double frame_duration) {
    std::vector<shared_ptr<ObjectFrame>> objectFrames{};
    std::vector<shared_ptr<AudioBlockFormat>> audioBlockFormat{std::begin(block), std::end(block)};
    int startIndex = 0;
    for (int i = 0; i < audioBlockFormat.size(); ++i) {
        if (audioBlockFormat[i]->duration > 0 && audioBlockFormat[i]->rStartTime == 0) {
            startIndex = i;
        }
    }
    shared_ptr<AudioBlockFormat> lastBlock = std::make_shared<AudioBlockFormat>();
    memcpy((void*)lastBlock.get(), (void*)audioBlockFormat[startIndex].get(), sizeof(AudioBlockFormat));
    if (lastBlock->cartesian == 0) {
        auto azimuth_ = (float) (lastBlock->azimuth / 180.0f * M_PI);
        auto elevation_ = (float) (lastBlock->elevation / 180.0f * M_PI);
        float x = lastBlock->distance * std::cos(elevation_) * std::sin(azimuth_);
        float y = lastBlock->distance * std::sin(elevation_);
        float z = lastBlock->distance * std::cos(elevation_) * std::cos(azimuth_);
        lastBlock->x = x;
        lastBlock->y = y;
        lastBlock->z = z;
    } else {
        float x = -lastBlock->x;
        float y = lastBlock->z;
        float z = lastBlock->y;
        lastBlock->x = x;
        lastBlock->y = y;
        lastBlock->z = z;
    }
    while (lastBlock->duration >= frame_duration) {
        auto frame = std::make_shared<ObjectFrame>();
        frame->gain = lastBlock->gain;
        frame->x = lastBlock->x;
        frame->y = lastBlock->y;
        frame->z = lastBlock->z;
        objectFrames.emplace_back(frame);

        lastBlock->rStartTime += frame_duration;
        lastBlock->duration -= frame_duration;
        lastBlock->x = frame->x;
        lastBlock->y = frame->y;
        lastBlock->z = frame->z;
    }
    for (int i = startIndex; i < audioBlockFormat.size(); ++i) {
        auto currentBlock = audioBlockFormat[i];
        if (currentBlock->cartesian == 0) {
            auto azimuth_ = (float) (currentBlock->azimuth / 180.0f * M_PI);
            auto elevation_ = (float) (currentBlock->elevation / 180.0f * M_PI);
            float x = currentBlock->distance * std::cos(elevation_) * std::sin(azimuth_);
            float y = currentBlock->distance * std::sin(elevation_);
            float z = currentBlock->distance * std::cos(elevation_) * std::cos(azimuth_);
            currentBlock->x = x;
            currentBlock->y = y;
            currentBlock->z = z;
        } else {
            float x = -currentBlock->x;
            float y = currentBlock->z;
            float z = currentBlock->y;
            currentBlock->x = x;
            currentBlock->y = y;
            currentBlock->z = z;
        }
        if (lastBlock->duration >= frame_duration) {
            int count = (int) (lastBlock->duration / frame_duration);
            float x_step = (currentBlock->x - lastBlock->x) / (float) count;
            float y_step = (currentBlock->y - lastBlock->y) / (float) count;
            float z_step = (currentBlock->z - lastBlock->z) / (float) count;
            while (lastBlock->duration >= frame_duration) {
                auto frame = std::make_shared<ObjectFrame>();
                frame->gain = lastBlock->gain;
                frame->x = lastBlock->x + x_step;
                frame->y = lastBlock->y + y_step;
                frame->z = lastBlock->z + z_step;
                objectFrames.emplace_back(frame);

                lastBlock->rStartTime += frame_duration;
                lastBlock->duration -= frame_duration;
                lastBlock->x = frame->x;
                lastBlock->y = frame->y;
                lastBlock->z = frame->z;
            }
        }
        lastBlock->duration += currentBlock->duration;
        lastBlock->x = currentBlock->x;
        lastBlock->y = currentBlock->y;
        lastBlock->z = currentBlock->z;
        lastBlock->gain = currentBlock->gain;
    }
    return objectFrames;
}

std::shared_ptr<ObjectFrame> Metadata::getNextObjectFrame(int index) {
    for (const auto &mo : metaDataObjects) {
        for (const auto &ms : mo->metaDataSources) {
            if (ms->trackIndex < 0) {
                std::cerr << "trackIndex positive" << std::endl;
                exit(-1);
            }
            if (ms->trackIndex == index && ms->objectFrames.size() > ms->current_object_frame) {
                return ms->objectFrames[ms->current_object_frame++];
            }
        }
    }
    return nullptr;
}

int Metadata::getAmbisonicOrder() const {
    return 7;
}

int Metadata::connectAudioTrack(const std::map<std::string, int> &uid_map) {
    for (auto &ao: metaDataObjects) {
        for (auto &as :ao->metaDataSources) {
            std::string &track_uid = as->audioTrackUID;
            if (uid_map.find(track_uid) != uid_map.end()) {
                as->trackIndex = uid_map.at(track_uid) - 1;
                if (as->trackIndex < 0) {
                    std::cerr << "trackIndex < 0 when AudioTrackUID is " << track_uid;
                    return -1;
                }
            } else {
                std::cerr << "can not connect audioTrackUID " << track_uid << " with audio track";
                return -1;
            }
        }
    }
    return 0;
}
}