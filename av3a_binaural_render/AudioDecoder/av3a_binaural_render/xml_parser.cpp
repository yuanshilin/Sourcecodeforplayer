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

#include "xml_parser.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace AVS3 {

std::shared_ptr<XmlNode> XmlParser::loadFromString(const std::string &xml_str) {
    std::string::size_type title_start_index = xml_str.find("<?");
    std::string::size_type title_end_index = xml_str.find("?>");
    if (title_start_index == std::string::npos || title_end_index == std::string::npos) {
        return nullptr;
    }
    std::shared_ptr<XmlNode> root = nullptr;
    buildXmlTree(root, const_cast<std::string &>(xml_str), (uint64_t)title_end_index + 2);
    return root;
}

std::shared_ptr<XmlNode> XmlParser::loadFromFile(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr;
    }
    std::string xml_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return loadFromString(xml_data);
}

uint64_t XmlParser::buildXmlTree(std::shared_ptr<XmlNode> &node, std::string &xml_str, uint64_t parse_index) {
    if (xml_str.size() <= parse_index + 1) {
        return 0;
    }

    uint64_t current_index = parse_index;
    std::string &parse_str = xml_str;
    while (true) {
        std::string line_str = parseLine(parse_str, current_index);
        if (line_str.empty()) {
            return 0;
        }
        if (is_comment_tag(line_str)) {
            uint64_t next_index =  getNextParseIndex(parse_str, current_index);
            if (next_index > 0) {
                current_index = next_index;
            }
            continue;
        }
        if (is_left_tag(line_str)) {
            std::shared_ptr<XmlNode> child;
            if (node == nullptr) {
                node = std::make_shared<XmlNode>();
                child = node;
            } else {
                child = std::make_shared<XmlNode>();
                node->children.push_back(child);
            }
            auto attrs = getAttribute(line_str, 0);
            assert(!attrs.empty());
            child->label_name = attrs["label_name"];
            attrs.erase("label_name");
            if (!attrs.empty()) {
                for (auto &attr : attrs) {
                    child->attributes[attr.first] = attr.second;
                }
            }
            uint64_t next_index =  getNextParseIndex(parse_str, current_index);
            if (next_index > 0) {
                current_index = next_index;
            }
            current_index = buildXmlTree(child, parse_str, current_index);
        } else if (is_full_tag(line_str)) {
            std::shared_ptr<XmlNode> child;
            if (node == nullptr) {
                node = std::make_shared<XmlNode>();
                child = node;
            } else {
                child = std::make_shared<XmlNode>();
                node->children.push_back(child);
            }
            auto attrs = getAttribute(line_str, 1);
            assert(!attrs.empty());
            child->label_name = attrs["label_name"];
            attrs.erase("label_name");
            if (!attrs.empty()) {
                for (auto &attr : attrs) {
                    child->attributes[attr.first] = attr.second;
                }
            }
            uint64_t next_index =  getNextParseIndex(parse_str, current_index);
            if (next_index > 0) {
                current_index = next_index;
            }
        } else if (is_right_tag(line_str)) {
            uint64_t next_index =  getNextParseIndex(parse_str, current_index);
            if (next_index - line_str.size() > current_index) {
                uint64_t val_end = next_index - line_str.size();
                assert(val_end > current_index);
                auto val_str = parse_str.substr(current_index, val_end - current_index);
                val_str.erase(remove_if(val_str.begin(), val_str.end(), ::isspace), val_str.end());
                val_str.erase(remove(val_str.begin(), val_str.end(), '\n'), val_str.end());
                if (!val_str.empty()) {
                    if (node) {
                        node->value = val_str;
                    }
                }
            }
            if (next_index > 0) {
                current_index = next_index;
            }
            return current_index;
        }
    }
}

std::map<std::string, std::string> XmlParser::getAttribute(std::string &sub_str, int tag) {
    auto attrs = splitAttr(sub_str, tag);
    assert(!attrs.empty());
    std::map<std::string, std::string> attr;
    attr["label_name"] = attrs[0];
    for (int i = 1; i < attrs.size(); ++i) {
        auto attr_str = attrs[i];
        std::string split = "=";
        auto attr_ret = splitString(attr_str, split);
        if (attr_ret.size() == 1) {
            attr[attr_ret[0]] = "";
        } else {
            assert(attr_ret.size() == 2);
            assert(attr_ret[1].size() >= 2);
            attr[attr_ret[0]] = attr_ret[1].substr(1, attr_ret[1].size() - 2);
        }
    }
    return attr;
}

std::vector<std::string> XmlParser::splitAttr(std::string &msg, int tag) {
    int end = tag == 1 ? 2 : 1;
    char split = ' ';
    assert(msg.size() > (end + 1));
    std::string vote_str = msg.substr(1, msg.size() - end - 1);

    int mark_count = 0;
    size_t pos1 = 0, pos2;
    size_t len = vote_str.length();
    std::vector<std::string> ret;
    for (size_t i = 0; i < len; i++) {
        char letter = vote_str[i];
        if (letter == '\n') {
            continue;
        }
        if (letter == ' ' && i + 1 < len && vote_str[i + 1] == ' ') {
            continue;
        }
        if (letter == '\"') {
            mark_count++;
        }
        if (letter == split && mark_count % 2 == 0) {
            pos2 = i;
            ret.emplace_back(vote_str.substr(pos1, pos2 - pos1));
            pos1 = pos2 + 1;
        }
    }

    if (pos1 != len) {
        ret.emplace_back(vote_str.substr(pos1));
    }
    return ret;
}

uint64_t XmlParser::getNextParseIndex(std::string &xml_str, uint64_t current_index) {
    std::string::size_type pos = xml_str.find_first_of('>', current_index);
    if (pos != std::string::npos) {
        return (uint64_t)pos + 1;
    }
    return 0;
}

std::string XmlParser::parseLine(std::string &xml_str, uint64_t current_index) {
    std::string::size_type left = xml_str.find_first_of('<', current_index);
    std::string::size_type right = xml_str.find_first_of('>', current_index);
    if (left == std::string::npos || right == std::string::npos) {
        return {};
    }
    std::string line_str = xml_str.substr(left, right - left + 1);
    return line_str;
}

bool XmlParser::is_left_tag(const std::string &label_str) {
    return label_str.find_first_of('<') == 0 && label_str.find("</") == std::string::npos && label_str.find_last_of('>') == label_str.size() - 1 && label_str.find("/>") == std::string::npos;
}

bool XmlParser::is_right_tag(const std::string &label_str) {
    return label_str.find("</") == 0 && label_str.find_last_of('>') == label_str.size() - 1 && label_str.find("/>") == std::string::npos;
}

bool XmlParser::is_full_tag(const std::string &label_str) {
    return label_str.find_first_of('<') == 0 && label_str.find("</") == std::string::npos && label_str.find("/>") == label_str.size() - 2;
}

bool XmlParser::is_comment_tag(const std::string &label_str) {
//    std::cout << "comment label len " <<  label_str.size() << " end index " << label_str.find_last_of("-->") << std::endl;
    return label_str.find("<!--") == 0 && label_str.find_last_of("-->") == label_str.size() - 1;
}

} // namespace AVS3
