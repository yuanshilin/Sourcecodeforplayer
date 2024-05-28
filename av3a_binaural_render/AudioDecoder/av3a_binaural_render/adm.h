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

#ifndef AVS3_RENDER_ADM_H
#define AVS3_RENDER_ADM_H

#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <list>

namespace AVS3 {

class AudioContent;

class AudioObject;

class AudioChannelFormat;

class AudioBlockFormat;

class AudioTrackFormat;

class AudioStreamFormat;

class AudioPackFormat;

class AudioTrackUID;

class ZoneExclusion;

class ADM;

enum TypeDefinition {
    DirectSpeakers = 1,
    Matrix,
    Object,
    HOA,
    Binaural
};

enum FormatDefinition {
    PCM = 1
};

struct LoudnessMetaData {
    std::string loudnessMethod{};
    std::string loudnessRecType{};
    std::string loudnessCorrectionType{};
    float integratedLoudness;
    float loudnessRange;
    float maxTruePeak;
    float maxMomentary;
    float maxShortTerm;
    float dialogueLoudness;
};

struct ReferenceScreen {
    float aspectRatio{};
    float centerPosX{};
    float centerPosY{};
    float centerPosZ{};
    float centerPosAzimuth{};
    float centerPosElevation{};
    float centerPosDistance{};
    float screenWidthX{};
    float screenWidthAzimuth{};
};

class AudioElement {
public:
    std::string id{};
    bool is_common_definition{false};

    virtual void lookupReference(ADM *adm) = 0;
};

class AudioProgram: public AudioElement {
public:
    std::string name{};
    double start{0};
    double end{0};
    int maxDuckingDepth{0};
    std::vector<std::shared_ptr<AudioContent>> audioContent{};
    std::vector<std::string> audioContentIDRef{};
    std::shared_ptr<LoudnessMetaData> loudnessMetaData{};
    std::shared_ptr<ReferenceScreen> referenceScreen{nullptr};

    void lookupReference(ADM *adm) override;
};

class AudioContent: public AudioElement {
public:
    std::string name{};
    std::shared_ptr<LoudnessMetaData> loudnessMetadata{};
    int dialogue{0};
    std::vector<std::shared_ptr<AudioObject>> audioObjects{};
    std::vector<std::string> audioObjectIDRef{};

    void lookupReference(ADM *adm) override ;
};

class AudioObjectInteraction {
public:
    int onOffInteract{};
    int gainInteract{};
    int positionInteract{};
    float gainInteractionRangeMin{};
    float gainInteractionRangeMax{};
    float positionInteractionRange_min_azimuth{};
    float positionInteractionRange_max_azimuth{};
    float positionInteractionRange_min_elevation{};
    float positionInteractionRange_max_elevation{};
    float positionInteractionRange_min_distance{};
    float positionInteractionRange_max_distance{};

};

class AudioObject: public AudioElement {
public:
    std::string name{};
    double start{0};
    double duration{0};
    int32_t importance{0};
    bool interact{false};
    bool disableDucking{false};
    int dialogue{0};
    std::vector<std::shared_ptr<AudioPackFormat>> audioPackFormats{};
    std::vector<std::shared_ptr<AudioTrackUID>> audioTrackUIDs{};
//    std::vector<std::shared_ptr<AudioObject>> audioObjects{};
//    std::vector<std::shared_ptr<AudioObject>> audioComplementaryObjects{};
    std::vector<std::string> audioPackFormatIDRef{};
    std::vector<std::string> audioTrackUIDRef{};
//    std::vector<std::string> audioObjectIDRef{};
    std::vector<std::string> audioComplementaryObjectIDRef{};
    std::shared_ptr<AudioObjectInteraction> audioObjectInteraction{nullptr};

    void lookupReference(ADM *adm) override;
};

class AudioPackFormat: public AudioElement {
public:
    std::string name{};
    TypeDefinition type;
    std::string typeLabel{};
    int32_t absoluteDistance{2};
    std::vector<std::shared_ptr<AudioChannelFormat>> audioChannelFormats{};
//    std::vector<std::shared_ptr<AudioPackFormat>> audioPackFormats{};
    int importance{0};

    // attributes for type==Matrix
    // encode and decode pack references are a single binary many-many
    // relationship; only store one side
    std::vector<std::shared_ptr<AudioPackFormat>> encodePackFormats{};
    std::vector<std::shared_ptr<AudioPackFormat>> decodePackFormats{};
    std::vector<std::shared_ptr<AudioPackFormat>> inputPackFormats{};
    std::vector<std::shared_ptr<AudioPackFormat>> outputPackFormats{};

    // attributes for type==HOA
    int normalization{};
    float nfcRefDist{0};
    int screenRef;

    std::vector<std::string> audioChannelFormatIDRef{};
//    std::vector<std::string> audioPackFormatIDRef{};

    //for matrix
    std::vector<std::string> encodePackFormatIDRef{};
    std::vector<std::string> decodePackFormatIDRef{};
    std::vector<std::string> inputPackFormatIDRef{};
    std::vector<std::string> outputPackFormatIDRef{};

    void lookupReference(ADM *adm) override;
};

class Frequency {
public:
    int lowPass{};
    int highPass{};
    int cutoffFrequency{};
};

class AudioChannelFormat: public AudioElement {
public:
    std::string name{};
    TypeDefinition type;
    std::string typeLabel{};
    std::list<std::shared_ptr<AudioBlockFormat>> audioBlockFormat{};
    std::vector<std::shared_ptr<Frequency>> frequency{};

    void lookupReference(ADM *adm) override;
};

class AudioTrackUID: public AudioElement {
public:
    int32_t trackIndex{0};
    int32_t sampleRate{0};
    int32_t bitDepth{0};
    std::vector<std::shared_ptr<AudioTrackFormat>> audioTrackFormat{};
    std::vector<std::shared_ptr<AudioPackFormat>> audioPackFormat{};
    std::vector<std::shared_ptr<AudioChannelFormat>> audioChannelFormat{};
    std::vector<std::string> audioTrackFormatIDRef{};
    std::vector<std::string> audioPackFormatIDRef{};
    std::vector<std::string> audioChannelFormatIDRef{};

    std::string packageUIDRef{};
    std::string trackIDRef{};
    std::string channelIDRef{};

    void lookupReference(ADM *adm) override;
};

class AudioTrackFormat: public AudioElement {
public:
    std::string name{};
    std::string formatLabel{};
    FormatDefinition format{};
    TypeDefinition type;
    std::string typeLabel{};
    std::shared_ptr<AudioStreamFormat> audioStreamFormat{nullptr};
    std::string audioStreamFormatIDRef{};

    void lookupReference(ADM *adm) override;
};

class AudioStreamFormat: public AudioElement {
public:
    std::string name{};
    FormatDefinition format{};
    std::string formatLabel{};
    TypeDefinition type;
    std::string typeLabel;
    std::vector<std::shared_ptr<AudioTrackFormat>> audioTrackFormat{};
    std::vector<std::shared_ptr<AudioChannelFormat>> audioChannelFormat{};
    std::vector<std::shared_ptr<AudioPackFormat>> audioPackFormat{};
    std::vector<std::string> audioTrackFormatIDRef{};
    std::vector<std::string> audioChannelFormatIDRef{};
    std::vector<std::string> audioPackFormatIDRef{};

    void lookupReference(ADM *adm) override;
};

class AudioBlockFormat: public AudioElement {
public:
    double rStartTime;
    double duration;

    //sub-element
    //common
    float gain{1};
    int importance{10};
    int headLocked;
    int headphoneVirtualize_bypass;
    int headphoneVirtualize_DRR;

    //for directSpeakers
    std::string speakerLabel{};
    float azimuth{0};
    float elevation{0};
    float distance{1};

    float x{0};
    float y{0};
    float z{0};

    //matrix
    std::vector<std::string> outputChannelFormatIDRef{};
    float objectDivergence{0};
    float jumpPosition{0};
    float azimuthRange{45};
    float interpolationLength;
    std::string gainVar;
    float phase;
    std::string phaseVar;
    float delay;
    std::string delayVar;
    std::vector<std::string> audioChannelFormatID{};

    //for object
    float width{0};
    float height{0};
    float depth{0};
    float diffuse{0};
    int channelLock{0};
    int cartesian{0};
    float maxDistance{2};

    //for HOA
    int order;
    int degree;
    std::string normalization{};
    int screenRef{0};
    int screenEdgeLock{0};
    float nfcDist;
    std::string equation;
    std::vector<std::shared_ptr<ZoneExclusion>> zoneExclusion{};

    void lookupReference(ADM *adm) override;
};

class ZoneExclusion {
public:
    float minElevation;
    float maxElevation;
    float minAzimuth;
    float maxAzimuth;
    std::string zoneValue;
};

class ADM {
public:

    void addAudioProgram(std::shared_ptr<AudioProgram> &ap);

    void addAudioContent(std::shared_ptr<AudioContent> &ac);

    void addAudioObject(std::shared_ptr<AudioObject> &ao);

    void addAudioPackFormat(std::shared_ptr<AudioPackFormat> &apf);

    void addAudioChannelFormat(std::shared_ptr<AudioChannelFormat> &acf);

    void addAudioStreamFormat(std::shared_ptr<AudioStreamFormat> &asf);

    void addAudioTrackFormat(std::shared_ptr<AudioTrackFormat> &atf);

    void addAudioTrackUID(std::shared_ptr<AudioTrackUID> &atu);

    void lookupReference();

    const std::vector<std::shared_ptr<AudioProgram>> &getAudioProgram() const;

    const std::vector<std::shared_ptr<AudioContent>> &getAudioContent() const;

    const std::vector<std::shared_ptr<AudioObject>> &getAudioObject() const;

    const std::vector<std::shared_ptr<AudioPackFormat>> &getAudioPackFormat() const;

    const std::vector<std::shared_ptr<AudioChannelFormat>> &getAudioChannelFormat() const;

    const std::vector<std::shared_ptr<AudioStreamFormat>> &getAudioStreamFormat() const;

    const std::vector<std::shared_ptr<AudioTrackFormat>> &getAudioTrackFormat() const;

    const std::vector<std::shared_ptr<AudioTrackUID>> &getAudioTrackUid() const;

    std::vector<std::string> toString() const;

private:
    std::vector<std::shared_ptr<AudioProgram>> audioProgram{};
    std::vector<std::shared_ptr<AudioContent>> audioContent{};
    std::vector<std::shared_ptr<AudioObject>> audioObject{};
    std::vector<std::shared_ptr<AudioPackFormat>> audioPackFormat{};
    std::vector<std::shared_ptr<AudioChannelFormat>> audioChannelFormat{};
    std::vector<std::shared_ptr<AudioStreamFormat>> audioStreamFormat{};
    std::vector<std::shared_ptr<AudioTrackFormat>> audioTrackFormat{};
    std::vector<std::shared_ptr<AudioTrackUID>> audioTrackUID{};
};
}

#endif // AVS3_RENDER_ADM_H
