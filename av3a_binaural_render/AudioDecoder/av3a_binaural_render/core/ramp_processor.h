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

#ifndef RAMP_PROCESSOR_H
#define RAMP_PROCESSOR_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <atomic>

namespace avs3renderer {

class RampProcessor {
public:
    ///
    enum RampMode { ConstantRate, ConstantDuration };

    /// RampProcessor has two mode:
    /// ConstantRate is used for gain ramp, it has constant increment per sample.
    /// ConstantDuration is used for doppler distance ramp, it has constant ramp length to change value to target.
    RampProcessor(RampMode ramp_mode = ConstantRate);

    RampProcessor& operator=(const RampProcessor& other);

    void SetTargetValue(const float value);

    int GetRampValue(std::vector<float>& values, const int num_frames);

    float target_value() const {
        return target_value_;
    }

    void SetScale(float scale) {
        scale_ = scale;
    }

private:
    RampMode ramp_mode_;

    bool initialized_;
    float target_value_;
    float current_value_;
    float increment_;
    int ramp_length_;

    float scale_;

    static constexpr float kDefaultUnitRampLength = 2048.0f;        // used in constant rate mode
    static constexpr float kDefaultRampIncrement = 1.0f / 2048.0f;  // used in constant rate mode
    static constexpr float kDefaultTotalRampLength = 10240;         // used in constant duration mode
    static constexpr float kDefaultMinimumRampIncrement = 0.001f;   // used in constant duration mode
};
}  // namespace avs3renderer

#endif  // RAMP_PROCESSOR_H
