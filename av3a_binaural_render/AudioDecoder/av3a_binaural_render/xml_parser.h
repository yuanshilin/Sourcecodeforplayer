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

#ifndef AVS3_RENDER_XML_PARSER_H
#define AVS3_RENDER_XML_PARSER_H

#include <map>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <cassert>
#include "utils.h"

namespace AVS3 {

class XmlNode {
public:
    std::string label_name {};
    std::string value {};
    std::map<std::string, std::string> attributes {};
    std::list<std::shared_ptr<XmlNode>> children {};
    std::shared_ptr<XmlNode> parent {nullptr};
};

class XmlParser {

public:

    static std::shared_ptr<XmlNode> loadFromFile(const std::string &path);

    static std::shared_ptr<XmlNode> loadFromString(const std::string &xml_str);


private:

    static inline std::vector<std::string> splitAttr(std::string &, int);

    static inline uint64_t buildXmlTree(std::shared_ptr<XmlNode> &, std::string &xml_str, uint64_t parse_index);

    static inline std::string parseLine(std::string &, uint64_t);

    static inline uint64_t getNextParseIndex(std::string &xml_str, uint64_t current_index);

    static inline std::map<std::string, std::string> getAttribute(std::string &sub_str, int tag);

    static inline bool is_left_tag(const std::string &label_str);

    static inline bool is_right_tag(const std::string &label_str);

    static inline bool is_full_tag(const std::string &label_str);

    static inline bool is_comment_tag(const std::string &label_str);
};

} // namespace AVS3

#endif // AVS3_RENDER_XML_PARSER_H
