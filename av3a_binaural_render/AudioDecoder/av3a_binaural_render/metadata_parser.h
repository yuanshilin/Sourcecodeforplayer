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

#ifndef AVS3_RENDER_METADATA_PARSER_H
#define AVS3_RENDER_METADATA_PARSER_H

#include "adm.h"
#include "xml_parser.h"
#include "metadata.h"
#include <string>
#include <cassert>

namespace AVS3 {

class MetadataParser {

public:

    static std::shared_ptr<Metadata> getMetadata(std::string &adm_xml, int sampleRate, int framePerBuffer);

    static std::shared_ptr<ADM> loadAdmFromString(std::string &xml_str);

private:

    static std::shared_ptr<ADM> loadAdmFromFile(std::string &path);

    static void parseElement(std::shared_ptr<ADM> &adm, std::shared_ptr<XmlNode> &node);

    static std::shared_ptr<AudioProgram> parseAudioProgram(std::shared_ptr<XmlNode> &node);

    static std::shared_ptr<AudioContent> parseAudioContent(std::shared_ptr<XmlNode> &node);

    static std::shared_ptr<AudioObject> parseAudioObject(std::shared_ptr<XmlNode> &node);

    static std::shared_ptr<AudioPackFormat> parseAudioPackFormat(std::shared_ptr<XmlNode> &node);

    static std::shared_ptr<AudioChannelFormat> parseAudioChannelFormat(std::shared_ptr<XmlNode> &node);

    static std::shared_ptr<AudioTrackFormat> parseAudioTrackFormat(std::shared_ptr<XmlNode> &node);

    static std::shared_ptr<AudioStreamFormat> parseAudioStreamFormat(std::shared_ptr<XmlNode> &node);

    static std::shared_ptr<AudioTrackUID> parseAudioTrackUID(std::shared_ptr<XmlNode> &node);

    static TypeDefinition getType(std::string &type);

    static float parseTimeStr2Ms(std::string &timeStr);
};
} // namespace AVS3

#endif // AVS3_RENDER_METADATA_PARSER_H
