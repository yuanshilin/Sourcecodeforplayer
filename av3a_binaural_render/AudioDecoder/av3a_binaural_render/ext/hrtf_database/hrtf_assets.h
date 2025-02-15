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

#ifndef THIRD_PARTY_HRTF_DATABASE_GENERATED_HRTF_ASSETS_H_
#define THIRD_PARTY_HRTF_DATABASE_GENERATED_HRTF_ASSETS_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace avs3renderer {

class HrtfAssets {
 private:
  typedef std::unordered_map<std::string, std::vector<std::vector<float>>> AssetDataMap;
public:
  static const AssetDataMap kAssetMap;
};

}  // namespace avs3renderer

#endif  // THIRD_PARTY_HRTF_DATABASE_GENERATED_HRTF_ASSETS_H_
