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

#include "metadata_parser.h"
#include "utils.h"
#include <iostream>
#include <fstream>

//#define PRINT_LOG 1

namespace AVS3 {

std::shared_ptr<Metadata> MetadataParser::getMetadata(std::string &adm_xml , int sampleRate, int framePerBuffer) {
    auto xmlRoot = XmlParser::loadFromString(adm_xml);
    if (xmlRoot == nullptr) {
        return nullptr;
    }
    auto adm = std::make_shared<ADM>();
    parseElement(adm, xmlRoot);
    adm->lookupReference();

    std::cout << "parse adm success" << std::endl;
    std::shared_ptr<Metadata> metaData = std::make_shared<Metadata>(adm, sampleRate, framePerBuffer);

#ifdef PRINT_LOG
    auto ret = adm->toString();
    ofstream file("metadata.txt");
    for (const auto &item : ret) {
        file << item << std::endl;
    }
#endif
    return metaData;
}

std::shared_ptr<ADM> MetadataParser::loadAdmFromString(std::string &xml_str) {
    auto xmlRoot = XmlParser::loadFromString(xml_str);
    if (xmlRoot == nullptr) {
        return nullptr;
    }
    auto adm = std::make_shared<ADM>();
    parseElement(adm, xmlRoot);
    adm->lookupReference();
    return adm;
}

std::shared_ptr<ADM> MetadataParser::loadAdmFromFile(std::string &path) {
    auto xmlRoot = XmlParser::loadFromFile(path);
    if (xmlRoot == nullptr) {
        return nullptr;
    }
    auto adm = std::make_shared<ADM>();
    parseElement(adm, xmlRoot);
    adm->lookupReference();
    return adm;
}

void MetadataParser::parseElement(std::shared_ptr<ADM> &adm, std::shared_ptr<XmlNode> &node) {
    if (node == nullptr) {
        return;
    }
    std::string &name = node->label_name;
    if (name == "audioProgramme") {
        auto ap = parseAudioProgram(node);
        adm->addAudioProgram(ap);
    } else if (name == "audioContent") {
        auto ac = parseAudioContent(node);
        adm->addAudioContent(ac);
    } else if (name == "audioObject") {
        auto ao = parseAudioObject(node);
        adm->addAudioObject(ao);
    } else if (name == "audioChannelFormat") {
        auto acf = parseAudioChannelFormat(node);
        adm->addAudioChannelFormat(acf);
    } else if (name == "audioPackFormat") {
        auto apf = parseAudioPackFormat(node);
        adm->addAudioPackFormat(apf);
    } else if (name == "audioStreamFormat") {
        auto asf = parseAudioStreamFormat(node);
        adm->addAudioStreamFormat(asf);
    } else if (name == "audioTrackFormat") {
        auto atf = parseAudioTrackFormat(node);
        adm->addAudioTrackFormat(atf);
    } else if (name == "audioTrackUID") {
        auto atu = parseAudioTrackUID(node);
        adm->addAudioTrackUID(atu);
    } else {
        for (auto &child: node->children) {
            parseElement(adm, child);
        }
    }
}

std::shared_ptr<AudioProgram> MetadataParser::parseAudioProgram(std::shared_ptr<XmlNode> &node) {
    auto ap = std::make_shared<AudioProgram>();
    auto attr = node->attributes;
    if (attr.find("audioProgrammeID") != attr.end()) {
        ap->id = attr["audioProgrammeID"];
    }
    if (attr.find("audioProgrammeName") != attr.end()) {
        ap->name = attr["audioProgrammeName"];
    }
    if (attr.find("start") != attr.end()) {
        ap->start = parseTimeStr2Ms(attr["start"]);
    }
    if (attr.find("end") != attr.end()) {
        ap->end = parseTimeStr2Ms(attr["end"]);
    }
    auto children = node->children;
    for (auto &child: children) {
        if (child->label_name == "audioContentIDRef") {
            ap->audioContentIDRef.push_back(child->value);
        } else if (child->label_name == "loudnessMetadata") {
            auto loudnessMetadata = std::make_shared<LoudnessMetaData>();
            auto loudAttr = child->attributes;
            if (loudAttr.find("loudnessMethod") != loudAttr.end()) {
                loudnessMetadata->loudnessMethod = loudAttr["loudnessMethod"];
            }
            if (loudAttr.find("loudnessRecType") != loudAttr.end()) {
                loudnessMetadata->loudnessRecType = loudAttr["loudnessRecType"];
            }
            if (loudAttr.find("loudnessCorrectionType") != loudAttr.end()) {
                loudnessMetadata->loudnessCorrectionType = loudAttr["loudnessCorrectionType"];
            }
            std::list<std::shared_ptr<XmlNode>> sub_children = child->children;
            for (auto &sub_child : sub_children) {
                if (sub_child->label_name == "integratedLoudness") {
                    loudnessMetadata->integratedLoudness = std::stof(sub_child->value);
                } else if (sub_child->label_name == "maxTruePeak") {
                    loudnessMetadata->maxTruePeak = std::stof(sub_child->value);
                } else if (sub_child->label_name == "loudnessRange") {
                    loudnessMetadata->loudnessRange = std::stof(sub_child->value);
                } else if (sub_child->label_name == "maxShortTerm") {
                    loudnessMetadata->maxShortTerm = std::stof(sub_child->value);
                } else if (sub_child->label_name == "maxMomentary") {
                    loudnessMetadata->maxMomentary = std::stof(sub_child->value);
                }
            }
            ap->loudnessMetaData = loudnessMetadata;
        } else if (child->label_name == "audioProgrammeReferenceScreen") {
            auto referenceScreen = std::make_shared<ReferenceScreen>();
            if (child->attributes.find("aspectRatio") != child->attributes.end()) {
                referenceScreen->aspectRatio = std::stof(child->attributes["aspectRatio"]);
            }
            for (auto &screen_child : child->children) {
                if (screen_child->label_name == "screenCentrePosition") {
                    auto sc_attr = screen_child->attributes;
                    if (sc_attr.find("azimuth") != sc_attr.end()) {
                        referenceScreen->centerPosAzimuth = std::stof(sc_attr["azimuth"]);
                    }
                    if (sc_attr.find("elevation") != sc_attr.end()) {
                        referenceScreen->centerPosElevation = std::stof(sc_attr["elevation"]);
                    }
                    if (sc_attr.find("distance") != sc_attr.end()) {
                        referenceScreen->centerPosDistance = std::stof(sc_attr["distance"]);
                    }
                    if (sc_attr.find("X") != sc_attr.end()) {
                        referenceScreen->centerPosX = std::stof(sc_attr["X"]);
                    }
                    if (sc_attr.find("Y") != sc_attr.end()) {
                        referenceScreen->centerPosY = std::stof(sc_attr["Y"]);
                    }
                    if (sc_attr.find("Z") != sc_attr.end()) {
                        referenceScreen->centerPosZ = std::stof(sc_attr["Z"]);
                    }
                }
                if (screen_child->label_name == "screenWidth") {
                    auto sc_attr = screen_child->attributes;
                    if (sc_attr.find("azimuth") != sc_attr.end()) {
                        referenceScreen->screenWidthAzimuth = std::stof(sc_attr["azimuth"]);
                    }
                    if (sc_attr.find("X") != sc_attr.end()) {
                        referenceScreen->screenWidthX = std::stof(sc_attr["X"]);
                    }
                }
            }
            ap->referenceScreen = referenceScreen;
        }
    }
    return ap;
}

std::shared_ptr<AudioContent> MetadataParser::parseAudioContent(std::shared_ptr<XmlNode> &node) {
    if (node == nullptr) {
        return nullptr;
    }
    auto ac = std::make_shared<AudioContent>();
    auto attr = node->attributes;
    if (attr.find("audioContentID") != attr.end()) {
        ac->id = attr["audioContentID"];
    }
    if (attr.find("audioContentName") != attr.end()) {
        ac->name = attr["audioContentName"];
    }
    auto children = node->children;
    for (auto &child: children) {
        if (child->label_name == "audioObjectIDRef") {
            ac->audioObjectIDRef.push_back(child->value);
        }
        if (child->label_name == "loudnessMetadata") {
            auto loudnessMetadata = std::make_shared<LoudnessMetaData>();
            auto loudAttr = child->attributes;
            if (loudAttr.find("loudnessMethod") != loudAttr.end()) {
                loudnessMetadata->loudnessMethod = loudAttr["loudnessMethod"];
            }
            if (loudAttr.find("loudnessRecType") != loudAttr.end()) {
                loudnessMetadata->loudnessRecType = loudAttr["loudnessRecType"];
            }
            if (loudAttr.find("loudnessCorrectionType") != loudAttr.end()) {
                loudnessMetadata->loudnessCorrectionType = loudAttr["loudnessCorrectionType"];
            }
            std::list<std::shared_ptr<XmlNode>> sub_children = child->children;
            for (auto &sub_child : sub_children) {
                if (sub_child->label_name == "integratedLoudness") {
                    loudnessMetadata->integratedLoudness = std::stof(sub_child->value);
                } else if (sub_child->label_name == "maxTruePeak") {
                    loudnessMetadata->maxTruePeak = std::stof(sub_child->value);
                } else if (sub_child->label_name == "loudnessRange") {
                    loudnessMetadata->loudnessRange = std::stof(sub_child->value);
                } else if (sub_child->label_name == "maxShortTerm") {
                    loudnessMetadata->maxShortTerm = std::stof(sub_child->value);
                } else if (sub_child->label_name == "maxMomentary") {
                    loudnessMetadata->maxMomentary = std::stof(sub_child->value);
                }
            }
            ac->loudnessMetadata = loudnessMetadata;
        }
        if (child->label_name == "dialogue") {
            ac->dialogue = std::stoi(child->value);
        }
    }
    return ac;
}

std::shared_ptr<AudioObject> MetadataParser::parseAudioObject(std::shared_ptr<XmlNode> &node) {
    if (node == nullptr) {
        return nullptr;
    }
    auto ao = std::make_shared<AudioObject>();
    auto attr = node->attributes;
    if (attr.find("audioObjectID") != attr.end()) {
        ao->id = attr["audioObjectID"];
    }
    if (attr.find("audioObjectName") != attr.end()) {
        ao->name = attr["audioObjectName"];
    }
    if (attr.find("start") != attr.end()) {
        ao->start = parseTimeStr2Ms(attr["start"]);
    }
    if (attr.find("duration") != attr.end()) {
        ao->duration = parseTimeStr2Ms(attr["duration"]);
    }
    if (attr.find("dialogue") != attr.end()) {
        ao->dialogue = std::stoi(attr["dialogue"]);
    }
    if (attr.find("importance") != attr.end()) {
        ao->importance = std::stoi(attr["importance"]);
    }
    if (attr.find("interact") != attr.end()) {
        ao->interact = std::stoi(attr["interact"]);
    }
    if (attr.find("disableDucking") != attr.end()) {
        ao->disableDucking = std::stoi(attr["disableDucking"]);
    }
    auto children = node->children;
    for (auto &child: children) {
        if (child->label_name == "audioPackFormatIDRef") {
            ao->audioPackFormatIDRef.push_back(child->value);
        } else if (child->label_name == "audioTrackUIDRef") {
            ao->audioTrackUIDRef.push_back(child->value);
        } else if (child->label_name == "audioComplementaryObjectIDRef") {
            ao->audioComplementaryObjectIDRef.push_back(child->value);
        } else if (child->label_name == "audioObjectInteraction") {
            auto audioObjectInteraction = std::make_shared<AudioObjectInteraction>();
            auto sub_attr = child->attributes;
            if (sub_attr.find("onOffInteract") != sub_attr.end()) {
                audioObjectInteraction->onOffInteract = std::stoi(sub_attr["onOffInteract"]);
            }
            if (sub_attr.find("gainInteract") != sub_attr.end()) {
                audioObjectInteraction->gainInteract = std::stoi(sub_attr["gainInteract"]);
            }
            if (sub_attr.find("positionInteract") != sub_attr.end()) {
                audioObjectInteraction->positionInteract = std::stoi(sub_attr["positionInteract"]);
            }
            auto sub_children = child->children;
            for (auto &sub_child : sub_children) {
                if (sub_child->label_name == "gainInteractionRange") {
                    if (sub_child->attributes["bound"] == "min") {
                        audioObjectInteraction->gainInteractionRangeMin = std::stof(sub_child->value);
                    } else if (sub_child->attributes["bound"] == "max") {
                        audioObjectInteraction->gainInteractionRangeMax = std::stof(sub_child->value);
                    }
                } else if (sub_child->label_name == "positionInteractionRange") {
                    if (sub_child->attributes["bound"] == "min") {
                        if (sub_child->attributes["coordinate"] == "azimuth") {
                            audioObjectInteraction->positionInteractionRange_min_azimuth = std::stof(sub_child->value);
                        } else if (sub_child->attributes["coordinate"] == "elevation") {
                            audioObjectInteraction->positionInteractionRange_min_elevation = std::stof(sub_child->value);
                        } else if (sub_child->attributes["coordinate"] == "distance") {
                            audioObjectInteraction->positionInteractionRange_min_distance = std::stof(sub_child->value);
                        }
                    } else if (sub_child->attributes["bound"] == "max") {
                        if (sub_child->attributes["coordinate"] == "azimuth") {
                            audioObjectInteraction->positionInteractionRange_max_azimuth = std::stof(sub_child->value);
                        } else if (sub_child->attributes["coordinate"] == "elevation") {
                            audioObjectInteraction->positionInteractionRange_max_elevation = std::stof(sub_child->value);
                        } else if (sub_child->attributes["coordinate"] == "distance") {
                            audioObjectInteraction->positionInteractionRange_max_distance = std::stof(sub_child->value);
                        }
                    }
                }
            }
            ao->audioObjectInteraction = audioObjectInteraction;
        }
    }
    return ao;
}

std::shared_ptr<AudioPackFormat> MetadataParser::parseAudioPackFormat(std::shared_ptr<XmlNode> &node) {
    if (node == nullptr) {
        return nullptr;
    }
    auto apf = std::make_shared<AudioPackFormat>();
    auto attr = node->attributes;
    if (attr.find("audioPackFormatID") != attr.end()) {
        apf->id = attr["audioPackFormatID"];
    }
    if (attr.find("audioPackFormatName") != attr.end()) {
        apf->name = attr["audioPackFormatName"];
    }
    if (attr.find("typeDefinition") != attr.end()) {
        auto type = attr["typeDefinition"];
        apf->type = getType(type);
    }
    if (attr.find("typeLabel") != attr.end()) {
        apf->typeLabel = attr["typeLabel"];
    }
    auto children = node->children;
    for (auto &child: children) {
        if (child->label_name == "audioChannelFormatIDRef") {
            apf->audioChannelFormatIDRef.push_back(child->value);
        } else if (child->label_name == "decodePackFormatIDRef") {
            apf->decodePackFormatIDRef.push_back(child->value);
        } else if (child->label_name == "inputPackFormatIDRef") {
            apf->inputPackFormatIDRef.push_back(child->value);
        } else if (child->label_name == "encodePackFormatIDRef") {
            apf->encodePackFormatIDRef.push_back(child->value);
        } else if (child->label_name == "outputPackFormatIDRef") {
            apf->outputPackFormatIDRef.push_back(child->value);
        } else if (child->label_name == "normalization") {
            apf->normalization = std::stoi(child->value);
        } else if (child->label_name == "nfcRefDist") {
            apf->nfcRefDist = std::stof(child->value);
        } else if (child->label_name == "screenRef") {
            apf->screenRef = std::stoi(child->value);
        }
    }
    return apf;
}

std::shared_ptr<AudioChannelFormat> MetadataParser::parseAudioChannelFormat(std::shared_ptr<XmlNode> &node) {
    if (node == nullptr) {
        return nullptr;
    }
    auto acf = std::make_shared<AudioChannelFormat>();
    auto attr = node->attributes;
    if (attr.find("audioChannelFormatID") != attr.end()) {
        acf->id = attr["audioChannelFormatID"];
    }
    if (attr.find("audioChannelFormatName") != attr.end()) {
        acf->name = attr["audioChannelFormatName"];
    }
    if (attr.find("typeDefinition") != attr.end()) {
        auto type = attr["typeDefinition"];
        acf->type = getType(type);
    }
    if (attr.find("typeLabel") != attr.end()) {
        acf->typeLabel = attr["typeLabel"];
    }
    auto children = node->children;
    for (auto &child: children) {
        if (child->label_name == "audioBlockFormat") {
            auto audioBlockFormat = std::make_shared<AudioBlockFormat>();
            auto sub_attr = child->attributes;
            if (sub_attr.find("audioBlockFormatID") != sub_attr.end()) {
                audioBlockFormat->id = sub_attr["audioBlockFormatID"];
            }
            if (sub_attr.find("rtime") != sub_attr.end()) {
                audioBlockFormat->rStartTime = parseTimeStr2Ms(sub_attr["rtime"]);
            }
            if (sub_attr.find("duration") != sub_attr.end()) {
                audioBlockFormat->duration = parseTimeStr2Ms(sub_attr["duration"]);
            }
            auto sub_children = child->children;
            for (auto &sub_child : sub_children) {
                if (sub_child->label_name == "speakerLabel") {
                    audioBlockFormat->speakerLabel = sub_child->value;
                } else if (sub_child->label_name == "cartesian") {
                    audioBlockFormat->cartesian = std::stoi(sub_child->value);
                } else if (sub_child->label_name == "position") {
                    if (sub_child->attributes.find("coordinate") != sub_child->attributes.end()) {
                        auto prop = sub_child->attributes["coordinate"];
                        if (prop == "azimuth") {
                            audioBlockFormat->azimuth = std::stof(sub_child->value);
                        } else if (prop == "elevation") {
                            audioBlockFormat->elevation = std::stof(sub_child->value);
                        } else if (prop == "distance") {
                            audioBlockFormat->distance = std::stof(sub_child->value);
                        } else if (prop == "X") {
                            audioBlockFormat->x = std::stof(sub_child->value);
                        } else if (prop == "Y") {
                            audioBlockFormat->y = std::stof(sub_child->value);
                        } else if (prop == "Z") {
                            audioBlockFormat->z = std::stof(sub_child->value);
                        }
                    } else if (sub_child->attributes.find("screenEdgeLock") != sub_child->attributes.end()) {
                        auto lock = sub_child->attributes["screenEdgeLock"];
                        int screenEdgeLock = 0;
                        if (lock == "top") {
                            screenEdgeLock = 0;
                        } else if (lock == "right") {
                            screenEdgeLock = 1;
                        } else if (lock == "bottom") {
                            screenEdgeLock = 2;
                        } else if (lock == "left") {
                            screenEdgeLock = 3;
                        }
                        audioBlockFormat->screenEdgeLock = screenEdgeLock;
                    }
                } else if (sub_child->label_name == "degree") {
                    audioBlockFormat->degree = std::stoi(sub_child->value);
                } else if (sub_child->label_name == "order") {
                    audioBlockFormat->order = std::stoi(sub_child->value);
                } else if (sub_child->label_name == "normalization") {
                    audioBlockFormat->normalization = sub_child->value;
                } else if (sub_child->label_name == "gain") {
                    audioBlockFormat->gain = std::stof(sub_child->value);
                } else if (sub_child->label_name == "diffuse") {
                    audioBlockFormat->diffuse = std::stof(sub_child->value);
                } else if (sub_child->label_name == "width") {
                    audioBlockFormat->width = std::stof(sub_child->value);
                } else if (sub_child->label_name == "height") {
                    audioBlockFormat->height = std::stof(sub_child->value);
                } else if (sub_child->label_name == "depth") {
                    audioBlockFormat->depth = std::stof(sub_child->value);
                } else if (sub_child->label_name == "screenRef") {
                    audioBlockFormat->screenRef = std::stoi(sub_child->value);
                } else if (sub_child->label_name == "zoneExclusion") {
                    for (auto &zone : sub_child->children) {
                        std::shared_ptr<ZoneExclusion> zoneExclusion = std::make_shared<ZoneExclusion>();
                        auto zone_attr = zone->attributes;
                        if (zone_attr.find("minElevation") != zone_attr.end()) {
                            zoneExclusion->minElevation = std::stof(zone_attr["minElevation"]);
                        }
                        if (zone_attr.find("maxElevation") != zone_attr.end()) {
                            zoneExclusion->maxElevation = std::stof(zone_attr["maxElevation"]);
                        }
                        if (zone_attr.find("minAzimuth") != zone_attr.end()) {
                            zoneExclusion->minAzimuth = std::stof(zone_attr["minAzimuth"]);
                        }
                        if (zone_attr.find("maxAzimuth") != zone_attr.end()) {
                            zoneExclusion->maxAzimuth = std::stof(zone_attr["maxAzimuth"]);
                        }
                        zoneExclusion->zoneValue = zone->value;
                        audioBlockFormat->zoneExclusion.push_back(zoneExclusion);
                    }
                } else if (sub_child->label_name == "cartesian") {
                    audioBlockFormat->cartesian = std::stoi(sub_child->value);
                } else if (sub_child->label_name == "channelLock") {
                    audioBlockFormat->channelLock = std::stoi(sub_child->value);
                    if (sub_child->attributes.find("maxDistance") != sub_child->attributes.end()) {
                        audioBlockFormat->maxDistance = std::stof(sub_child->attributes["maxDistance"]);
                    }
                } else if (sub_child->label_name == "objectDivergence") {
                    audioBlockFormat->objectDivergence = std::stof(sub_child->value);
                    if (sub_child->attributes.find("azimuthRange") != sub_child->attributes.end()) {
                        audioBlockFormat->azimuthRange = std::stof(sub_child->attributes["azimuthRange"]);
                    }
                } else if (sub_child->label_name == "jumpPosition") {
                    audioBlockFormat->jumpPosition = std::stof(sub_child->value);
                    if (sub_child->attributes.find("interpolationLength") != sub_child->attributes.end()) {
                        audioBlockFormat->interpolationLength = std::stof(sub_child->attributes["interpolationLength"]);
                    }
                } else if (sub_child->label_name == "importance") {
                    audioBlockFormat->importance = std::stoi(sub_child->value);
                } else if (sub_child->label_name == "matrix") {
                    for (auto &matrix_child : sub_child->children) {
                        if (matrix_child->label_name == "coefficient") {
                            auto coe_attr = matrix_child->attributes;
                            audioBlockFormat->audioChannelFormatID.push_back(matrix_child->value);
                            if (coe_attr.find("gain") != coe_attr.end()) {
                                audioBlockFormat->gain = std::stof(coe_attr["gain"]);
                            }
                            if (coe_attr.find("phase") != coe_attr.end()) {
                                audioBlockFormat->phase = std::stof(coe_attr["phase"]);
                            }
                            if (coe_attr.find("delay") != coe_attr.end()) {
                                audioBlockFormat->delay = std::stof(coe_attr["delay"]);
                            }
                            if (coe_attr.find("gainVar") != coe_attr.end()) {
                                audioBlockFormat->gainVar = coe_attr["gainVar"];
                            }
                            if (coe_attr.find("phaseVar") != coe_attr.end()) {
                                audioBlockFormat->phaseVar = coe_attr["phaseVar"];
                            }
                            if (coe_attr.find("delayVar") != coe_attr.end()) {
                                audioBlockFormat->delayVar = coe_attr["delayVar"];
                            }
                        }
                    }
                } else if (sub_child->label_name == "outputChannelIDRef") {
                    audioBlockFormat->outputChannelFormatIDRef.push_back(sub_child->value);
                } else if (sub_child->label_name == "equation") {
                    audioBlockFormat->equation = sub_child->value;
                } else if (sub_child->label_name == "nfcRefDist") {
                    audioBlockFormat->nfcDist = std::stof(sub_child->value);
                }
            }
            acf->audioBlockFormat.push_back(audioBlockFormat);
        } else if (child->label_name == "frequency") {
            auto f = std::make_shared<Frequency>();
            f->cutoffFrequency = std::stoi(child->value);
            if (child->attributes.find("typeDefinition") != child->attributes.end()) {
                auto typeF = child->attributes["typeDefinition"];
                if (typeF == "lowPass") {
                    f->lowPass = 1;
                    f->highPass = 0;
                } else if (typeF == "highPass") {
                    f->highPass = 1;
                    f->lowPass = 0;
                }
            }
            acf->frequency.push_back(f);
        }
    }
    return acf;
}

std::shared_ptr<AudioTrackFormat> MetadataParser::parseAudioTrackFormat(std::shared_ptr<XmlNode> &node) {
    if (node == nullptr) {
        return nullptr;
    }
    auto atf = std::make_shared<AudioTrackFormat>();
    auto attr = node->attributes;
    if (attr.find("audioTrackFormatID") != attr.end()) {
        atf->id = attr["audioTrackFormatID"];
    }
    if (attr.find("audioTrackFormatName") != attr.end()) {
        atf->name = attr["audioTrackFormatName"];
    }
    if (attr.find("formatDefinition") != attr.end()) {
        atf->format = FormatDefinition::PCM;//attr["formatDefinition"];
    }
    if (attr.find("formatLabel") != attr.end()) {
        atf->formatLabel = attr["formatLabel"];
    }
    if (attr.find("typeLabel") != attr.end()) {
        atf->typeLabel = attr["typeLabel"];
    }
    if (attr.find("typeDefinition") != attr.end()) {
        atf->type = getType(attr["typeDefinition"]);
    }
    auto children = node->children;
    for (auto &child: children) {
        if (child->label_name == "audioStreamFormatIDRef") {
            atf->audioStreamFormatIDRef = child->value;
        }
    }
    return atf;
}

std::shared_ptr<AudioStreamFormat> MetadataParser::parseAudioStreamFormat(std::shared_ptr<XmlNode> &node) {
    if (node == nullptr) {
        return nullptr;
    }
    auto asf = std::make_shared<AudioStreamFormat>();
    auto attr = node->attributes;
    if (attr.find("audioStreamFormatID") != attr.end()) {
        asf->id = attr["audioStreamFormatID"];
    }
    if (attr.find("audioStreamFormatName") != attr.end()) {
        asf->name = attr["audioStreamFormatName"];
    }
    if (attr.find("formatDefinition") != attr.end()) {
        asf->format = FormatDefinition::PCM;//attr["formatDefinition"];
    }
    if (attr.find("typeLabel") != attr.end()) {
        asf->typeLabel = attr["typeLabel"];
    }
    if (attr.find("formatLabel") != attr.end()) {
        asf->formatLabel = attr["formatLabel"];
    }
    if (attr.find("typeDefinition") != attr.end()) {
        asf->type = getType(attr["typeDefinition"]);
    }
    auto children = node->children;
    for (auto &child: children) {
        if (child->label_name == "audioChannelFormatIDRef") {
            asf->audioChannelFormatIDRef.push_back(child->value);
        }
        if (child->label_name == "audioTrackFormatIDRef") {
            asf->audioTrackFormatIDRef.push_back(child->value);
        }
        if (child->label_name == "audioPackFormatIDRef") {
            asf->audioPackFormatIDRef.push_back(child->value);
        }
    }
    return asf;
}

std::shared_ptr<AudioTrackUID> MetadataParser::parseAudioTrackUID(std::shared_ptr<XmlNode> &node) {
    if (node == nullptr) {
        return nullptr;
    }
    auto atu = std::make_shared<AudioTrackUID>();
    auto attr = node->attributes;
    if (attr.find("UID") != attr.end()) {
        atu->id = attr["UID"];
    }
    if (attr.find("bitDepth") != attr.end()) {
        atu->bitDepth = std::stoi(attr["bitDepth"]);
    }
    if (attr.find("sampleRate") != attr.end()) {
        atu->sampleRate = std::stoi(attr["sampleRate"]);
    }
    auto children = node->children;
    for (auto &child: children) {
        if (child->label_name == "audioTrackFormatIDRef") {
            atu->audioTrackFormatIDRef.push_back(child->value);
        } else if (child->label_name == "audioPackFormatIDRef") {
            atu->audioPackFormatIDRef.push_back(child->value);
        } else if (child->label_name == "audioChannelFormatIDRef") {
            atu->audioChannelFormatIDRef.push_back(child->value);
        }
        if (child->label_name == "audioMXFLookUp") {
            auto sub_children = child->children;
            for (auto &sub_child : sub_children) {
                if (sub_child->label_name == "packageUIDRef") {
                    atu->packageUIDRef = sub_child->value;
                } else if (sub_child->label_name == "trackIDRef") {
                    atu->trackIDRef = sub_child->value;
                } else if (sub_child->label_name == "channelIDRef") {
                    atu->channelIDRef = sub_child->value;
                }
            }
        }
    }
    return atu;
}

TypeDefinition MetadataParser::getType(std::string &type) {
    if (type == "DirectSpeakers") {
        return TypeDefinition::DirectSpeakers;
    } else if (type == "Matrix") {
        return TypeDefinition::Matrix;
    } else if (type == "HOA") {
        return TypeDefinition::HOA;
    } else if (type == "Binaural") {
        return TypeDefinition::Binaural;
    } else if (type == "Objects") {
        return TypeDefinition::Object;
    } else {
        exit(-1);
    }
}

float MetadataParser::parseTimeStr2Ms(std::string &timeStr) {
    std::string split = ":";
    auto timeArr = splitString(timeStr, split);
    if (timeArr.size() != 3) {
        return -1;
    }
    float ret = std::stof(timeArr[0]) * 60 * 60 * 1000 + std::stof(timeArr[1]) * 60 * 1000 + std::stof(timeArr[2]) * 1000;
    return ret;
}
}