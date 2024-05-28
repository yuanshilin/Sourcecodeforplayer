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

#ifndef AVS3_RENDER_UTILS_H
#define AVS3_RENDER_UTILS_H

inline std::vector<std::string> splitString(std::string &msg, std::string &split) {
    std::string::size_type pos1, pos2;
    size_t len = msg.length();
    const std::string &delim = split;
    pos2 = msg.find(delim);
    pos1 = 0;
    std::vector<std::string> ret;
    while (std::string::npos != pos2) {
        ret.emplace_back(msg.substr(pos1, pos2 - pos1));
        pos1 = pos2 + delim.size();
        pos2 = msg.find(delim, pos1);
    }

    if (pos1 != len) {
        ret.emplace_back(msg.substr(pos1));
    }
    return ret;
}

#endif //AVS3_RENDER_UTILS_H
