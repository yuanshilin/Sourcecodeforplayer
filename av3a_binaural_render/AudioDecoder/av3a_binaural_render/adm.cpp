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

#include "adm.h"
#include <string>

namespace AVS3 {

void AudioProgram::lookupReference(ADM *metaData) {
    for (auto &contentRef : audioContentIDRef) {
        for (auto &content: metaData->getAudioContent()) {
            if (content->id == contentRef) {
                audioContent.push_back(content);
            }
        }
    }
}

void AudioContent::lookupReference(ADM *metaData) {
    for (auto &objectRef : audioObjectIDRef) {
        for (auto &object: metaData->getAudioObject()) {
            if (object->id == objectRef) {
                audioObjects.push_back(object);
            }
        }
    }
}

void AudioObject::lookupReference(ADM *metaData) {
    for (auto &packRef : audioPackFormatIDRef) {
        for (auto &packFormat: metaData->getAudioPackFormat()) {
            if (packFormat->id == packRef) {
                audioPackFormats.push_back(packFormat);
            }
        }
    }
    for (auto &atukRef : audioTrackUIDRef) {
        for (auto &atu: metaData->getAudioTrackUid()) {
            if (atu->id == atukRef) {
                audioTrackUIDs.push_back(atu);
            }
        }
    }
}

void AudioPackFormat::lookupReference(ADM *metaData) {
    for (auto &ackRef : audioChannelFormatIDRef) {
        for (auto &ac: metaData->getAudioChannelFormat()) {
            if (ac->id == ackRef) {
                audioChannelFormats.push_back(ac);
            }
        }
    }
    for (auto &aepRef : encodePackFormatIDRef) {
        for (auto &ap: metaData->getAudioPackFormat()) {
            if (ap->id == aepRef) {
                encodePackFormats.push_back(ap);
            }
        }
    }
    for (auto &adpRef : decodePackFormatIDRef) {
        for (auto &ap: metaData->getAudioPackFormat()) {
            if (ap->id == adpRef) {
                decodePackFormats.push_back(ap);
            }
        }
    }
    for (auto &aipRef : inputPackFormatIDRef) {
        for (auto &ap: metaData->getAudioPackFormat()) {
            if (ap->id == aipRef) {
                inputPackFormats.push_back(ap);
            }
        }
    }
    for (auto &aopRef : outputPackFormatIDRef) {
        for (auto &ap: metaData->getAudioPackFormat()) {
            if (ap->id == aopRef) {
                outputPackFormats.push_back(ap);
            }
        }
    }
}

void AudioChannelFormat::lookupReference(ADM *metaData) {
}

void AudioTrackUID::lookupReference(ADM *metaData) {
    for (auto &atRef: audioTrackFormatIDRef) {
        for (auto &at: metaData->getAudioTrackFormat()) {
            if (at->id == atRef) {
                audioTrackFormat.push_back(at);
            }
        }
    }
    for (auto &apRef: audioPackFormatIDRef) {
        for (auto &ap: metaData->getAudioPackFormat()) {
            if (ap->id == apRef) {
                audioPackFormat.push_back(ap);
            }
        }
    }
    for (auto &acRef: audioChannelFormatIDRef) {
        for (auto &ac: metaData->getAudioChannelFormat()) {
            if (ac->id == acRef) {
                audioChannelFormat.push_back(ac);
            }
        }
    }
}

void AudioTrackFormat::lookupReference(ADM *metaData) {
    for (auto &as: metaData->getAudioStreamFormat()) {
        if (as->id == audioStreamFormatIDRef) {
            audioStreamFormat = as;
        }
    }
}

void AudioStreamFormat::lookupReference(ADM *metaData) {
    for (auto &acRef : audioChannelFormatIDRef) {
        for (auto &ac: metaData->getAudioChannelFormat()) {
            if (ac->id == acRef) {
                audioChannelFormat.push_back(ac);
            }
        }
    }
    for (auto &apRef :audioPackFormatIDRef) {
        for (auto &ap: metaData->getAudioPackFormat()) {
            if (ap->id == apRef) {
                audioPackFormat.push_back(ap);
            }
        }
    }
    for (auto &atRef :audioTrackFormatIDRef) {
        for (auto &at: metaData->getAudioTrackFormat()) {
            if (at->id == atRef) {
                audioTrackFormat.push_back(at);
            }
        }
    }
}

void AudioBlockFormat::lookupReference(ADM *metaData) {
}

void ADM::lookupReference() {
    for (auto &child: audioProgram) {
        child->lookupReference(this);
    }
    for (auto &child: audioContent) {
        child->lookupReference(this);
    }
    for (auto &child: audioObject) {
        child->lookupReference(this);
    }
    for (auto &child: audioPackFormat) {
        child->lookupReference(this);
    }
    for (auto &child: audioChannelFormat) {
        child->lookupReference(this);
    }
    for (auto &child: audioStreamFormat) {
        child->lookupReference(this);
    }
    for (auto &child: audioTrackFormat) {
        child->lookupReference(this);
    }
    for (auto &child: audioTrackUID) {
        child->lookupReference(this);
    }
}

void ADM::addAudioProgram(std::shared_ptr<AudioProgram> &ap) {
    audioProgram.push_back(ap);
}

void ADM::addAudioContent(std::shared_ptr<AudioContent> &ac) {
    audioContent.push_back(ac);
}

void ADM::addAudioObject(std::shared_ptr<AudioObject> &ao) {
    audioObject.push_back(ao);
}

void ADM::addAudioPackFormat(std::shared_ptr<AudioPackFormat> &apf) {
    audioPackFormat.push_back(apf);
}

void ADM::addAudioChannelFormat(std::shared_ptr<AudioChannelFormat> &acf) {
    audioChannelFormat.push_back(acf);
}

void ADM::addAudioTrackFormat(std::shared_ptr<AudioTrackFormat> &atf) {
    audioTrackFormat.push_back(atf);
}

void ADM::addAudioTrackUID(std::shared_ptr<AudioTrackUID> &atu) {
    audioTrackUID.push_back(atu);
}

void ADM::addAudioStreamFormat(std::shared_ptr<AudioStreamFormat> &asf) {
    audioStreamFormat.push_back(asf);
}

const std::vector<std::shared_ptr<AudioProgram>> &ADM::getAudioProgram() const {
    return audioProgram;
}

const std::vector<std::shared_ptr<AudioContent>> &ADM::getAudioContent() const {
    return audioContent;
}

const std::vector<std::shared_ptr<AudioObject>> &ADM::getAudioObject() const {
    return audioObject;
}

const std::vector<std::shared_ptr<AudioPackFormat>> &ADM::getAudioPackFormat() const {
    return audioPackFormat;
}

const std::vector<std::shared_ptr<AudioChannelFormat>> &ADM::getAudioChannelFormat() const {
    return audioChannelFormat;
}

const std::vector<std::shared_ptr<AudioStreamFormat>> &ADM::getAudioStreamFormat() const {
    return audioStreamFormat;
}

const std::vector<std::shared_ptr<AudioTrackFormat>> &ADM::getAudioTrackFormat() const {
    return audioTrackFormat;
}

const std::vector<std::shared_ptr<AudioTrackUID>> &ADM::getAudioTrackUid() const {
    return audioTrackUID;
}

std::vector<std::string> ADM::toString() const {
    std::vector<std::string> ret;
    for (const auto &item : audioProgram) {
        ret.emplace_back("AudioProgramme:");
        ret.emplace_back(std::string("id:") + std::string(item->id));
        ret.emplace_back(std::string("name") + item->name);
        for (const auto &ac : item->audioContentIDRef) {
            ret.emplace_back("audioContentIDRef" + ac);
        }
    }
    for (const auto &item : audioContent) {
        ret.emplace_back("AudioContent:");
        ret.emplace_back("id:" + item->id);
        ret.emplace_back("name:" + item->name);
        for (const auto &ao : item->audioObjectIDRef) {
            ret.emplace_back("audioObjectRefID:" + ao);
        }
    }
    for (const auto &item : audioObject) {
        ret.emplace_back("AudioObject:");
        ret.emplace_back("id:" + item->id);
        ret.emplace_back("name:" + item->name);
        ret.emplace_back("start:" + std::to_string(item->start));
        ret.emplace_back("duration:" + std::to_string(item->duration));
        for (const auto &ap : item->audioPackFormatIDRef) {
            ret.emplace_back("audioPackFormatRefID:" + ap);
        }
        for (const auto &atu : item->audioTrackUIDRef) {
            ret.emplace_back("audioTrackUIDRefID:" + atu);
        }
    }
    for (const auto &item : audioPackFormat) {
        ret.emplace_back("AudioPackFormat:");
        ret.emplace_back("id:" + item->id);
        ret.emplace_back("name:" + item->name);
        ret.emplace_back("type:" + std::to_string((int)item->type));
        ret.emplace_back("typeLabel:" + item->typeLabel);
        for (const auto &ac : item->audioChannelFormatIDRef) {
            ret.emplace_back("audioChannelFormatRefID:" + ac);
        }
    }
    for (const auto &item : audioChannelFormat) {
        ret.emplace_back("AudioChannelFormat:");
        ret.emplace_back("id:" + item->id);
        ret.emplace_back("name:" + item->name);
        ret.emplace_back("type:" + std::to_string((int)item->type));
        ret.emplace_back("typeLabel:" + item->typeLabel);

        for (const auto &ab : item->audioBlockFormat) {
            ret.emplace_back("AudioBlockFormat:");
            ret.emplace_back("audioBlockFormatID:" + ab->id);
            ret.emplace_back("start:" + std::to_string(ab->rStartTime));
            ret.emplace_back("duration:" + std::to_string(ab->duration));
            ret.emplace_back("cartesian:" + std::to_string(ab->cartesian));
            ret.emplace_back("x,y,z:" + std::to_string(ab->x) + "," + std::to_string(ab->y) + "," + std::to_string(ab->z));
            ret.emplace_back("azi,ele,dis:" + std::to_string(ab->azimuth) + "," + std::to_string(ab->elevation) + "," + std::to_string(ab->distance));
        }
    }
    for (const auto &item : audioStreamFormat) {
        ret.emplace_back("AudioStreamFormat:");
        ret.emplace_back("id:" + item->id);
        ret.emplace_back("name:" + item->name);
        ret.emplace_back("formatDefinition:PCM");
        ret.emplace_back("formatLabel:" + item->formatLabel);
        for (const auto &at : item->audioTrackFormatIDRef) {
            ret.emplace_back("audioTrackFormatRefID:" + at);
        }
        for (const auto &ac : item->audioChannelFormatIDRef) {
            ret.emplace_back("audioChannelFormatRefID:" + ac);
        }
    }
    for (const auto &item : audioTrackFormat) {
        ret.emplace_back("AudioTrackFormat:");
        ret.emplace_back("id:" + item->id);
        ret.emplace_back("name:" + item->name);
        ret.emplace_back("formatDefinition:PCM");
        ret.emplace_back("formatLabel:" + item->formatLabel);
        ret.emplace_back("audioStreamFormatIDRef" + item->audioStreamFormatIDRef);
    }
    for (const auto &item : audioTrackUID) {
        ret.emplace_back("AudioTrackUID:");
        ret.emplace_back("id:" + item->id);
        ret.emplace_back("sampleRate:" + std::to_string(item->sampleRate));
        ret.emplace_back("bitDepth:" + std::to_string(item->bitDepth));
        for (const auto &ap : item->audioPackFormatIDRef) {
            ret.emplace_back("audioPackFormatRefID:" + ap);
        }
        for (const auto &at : item->audioTrackFormatIDRef) {
            ret.emplace_back("audioTrackFormatRefID:" + at);
        }
    }
    return ret;
}
}