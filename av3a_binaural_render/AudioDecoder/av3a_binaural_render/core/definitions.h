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

#pragma once
#include <Eigen/Dense>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <limits>
#include <chrono>

namespace avs3renderer {
//  Linear algebra type
using Point3f = Eigen::Vector3f;
using Vector3f = Eigen::Vector3f;
using Transform4f = Eigen::Matrix4f;

//  Complex type
using Complex = std::complex<float>;

using SampleRate = int;

// Air attenuation data calculated from Table 2 in
// https://www.jstage.jst.go.jp/article/souonseigyo1977/21/3/21_3_130/_pdf.
// a = log(10^(b/10))/1000
// a: air attenuation constant in inverse meters.
// b: Pure-tone atmospheric-absorption attenuation coefficients, in decibels per kilometre defined in the paper.

#if defined(BUILD_ANDROID) || defined(BUILD_IOS)

#ifdef __ANDROID__
#include <android/log.h>
#define TAG "abcd"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#endif
#define FOUR_OCTAVE_BAND
#elif defined(BUILD_WIN) || defined(BUILD_MAC) || defined(BUILD_LINUX)
#define FOUR_OCTAVE_BAND
#endif
#if defined(EIGHT_OCTAVE_BAND)
const size_t kNumOctaveBands = 8;
const float kOctaveBandCentres[kNumOctaveBands] = {62.5f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f};
const float kAirAttenuationConstant[kNumOctaveBands] = {0.00004, 0.00012, 0.00032, 0.00061,
                                                        0.00107, 0.00258, 0.00831, 0.02947};
#elif defined(FOUR_OCTAVE_BAND)
const size_t kNumOctaveBands = 4;
const float kOctaveBandCentres[kNumOctaveBands] = {1000.0f, 2000.0f, 4000.0f, 8000.0f};
const float kAirAttenuationConstant[kNumOctaveBands] = {0.00107, 0.00258, 0.00831, 0.02947};
#endif


const float kDegreeFromRadian = 180.f / M_PI;
const float kFloatTolerance = 1e-4f;
static const float kRotationQuantizationRad = 1.0f * M_PI / 180.f;

const int kNumStereoChannels = 2;

// At 20 °C (68 °F), the speed of sound in air is about 343 metres per second.
// Refer: https://en.wikipedia.org/wiki/Speed_of_sound
constexpr float kSpeedOfSound = 343.f;

inline const std::string ToString(const Eigen::Vector3f& v) {
    return "( " + std::to_string(v.x()) + ", " + std::to_string(v.y()) + ", " + std::to_string(v.z()) + ")";
}
}  // namespace avs3renderer
