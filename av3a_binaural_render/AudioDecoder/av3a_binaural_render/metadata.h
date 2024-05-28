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

#ifndef AVS3_RENDER_METADATA_H
#define AVS3_RENDER_METADATA_H

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <map>
#include "adm.h"

using namespace std;

namespace AVS3 {

struct ObjectFrame {
    float gain{1};
    float x{0};
    float y{0};
    float z{0};
};

struct DirectSpeakerFrame {
    float gain{1};
    float x{0};
    float y{0};
    float z{0};
};

struct HOARenderFrame {
    int order{0};
    int degree{0};
    int normalization{0};
    float gain{1};
};

struct MatrixFrame {
    vector<float> matrix{};
};

struct BinauralFrame {
    int channel_index{};
};

class MetadataSource {
public:
    int sampleRate;
    int bitDepth;
    AVS3::TypeDefinition type;
    int64_t trackIndex{-1};
    std::string audioTrackUID{};
    shared_ptr<DirectSpeakerFrame> directSpeakerFrame{};
    vector<shared_ptr<ObjectFrame>> objectFrames{};
    shared_ptr<HOARenderFrame> hoaFrame{};
    shared_ptr<BinauralFrame> binauralFrame{};
    shared_ptr<MatrixFrame> matrixFrame{};
    int64_t current_object_frame{0};
};

class MetadataObject {
public:
    int importance;
    int dialogue;
    int interact;
    int disableDucking;
    double start{0};
    double duration{0};
    vector<shared_ptr<MetadataSource>> metaDataSources{};
};

class MetadataContent {
public:
    string id{};
    string name{};
    int dialogue;
    shared_ptr<AVS3::LoudnessMetaData> loudnessMetaData{nullptr};
    vector<std::string> metaDataObjectsRefID{};
};

class MetadataProgram {
public:
    string name{};
    string id{};
    long startMs;
    long endMs;
    float maxDuckingDepth;
    string language{};
    shared_ptr<AVS3::LoudnessMetaData> loudnessMetaData{nullptr};
    vector<shared_ptr<MetadataContent>> metaDataContents{};
};

class Metadata {

public:
    string title{};
    string organisationName{};
    string dc_description{};
    string date{};
    string version{};
    int framePerBuffer;
    int sampleRate;
    vector<shared_ptr<MetadataProgram>> metaDataProgram{};
    vector<shared_ptr<MetadataObject>> metaDataObjects{};

public:

    explicit Metadata(shared_ptr<AVS3::ADM>&, int, int);

    std::shared_ptr<ObjectFrame> getNextObjectFrame(int index);

    int getNextListenerFrame(std::vector<float>& position, std::vector<float>& front, std::vector<float>& up) const;

    int getAmbisonicOrder() const;

    int connectAudioTrack(const std::map<std::string, int> &uid_map);

private:

    inline std::shared_ptr<MetadataObject> convertObject(std::shared_ptr<AVS3::AudioObject>&, float) const;

    static inline std::vector<shared_ptr<ObjectFrame>> parseObjectFrame(list<shared_ptr<AudioBlockFormat>> &, double);

};
}


#endif //AVS3_RENDER_METADATA_H
