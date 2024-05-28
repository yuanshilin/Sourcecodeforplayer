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

#ifndef SPHERICAL_HARMONICS_CONSTANTS_H
#define SPHERICAL_HARMONICS_CONSTANTS_H


namespace avs3renderer {

/** sqrt(4pi) (single precision) */
#define SQRT4PI (3.544907701811032f)

static const int kNumAmbisonicChannels[8] = {1, 4, 9, 16, 25, 36, 49, 64};

}  // namespace avs3renderer
#endif  // SPHERICAL_HARMONICS_CONSTANTS_H
