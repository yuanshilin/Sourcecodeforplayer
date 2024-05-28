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

#include "ramp_processor.h"

namespace avs3renderer {
RampProcessor::RampProcessor(RampMode ramp_mode)
    : ramp_mode_(ramp_mode),
      initialized_(false),
      target_value_(0.0f),
      current_value_(0.0f),
      increment_(0.0),
      ramp_length_(0),
      scale_(1.0) {
}

RampProcessor& RampProcessor::operator=(const RampProcessor& other) {
    ramp_mode_ = other.ramp_mode_;
    initialized_ = other.initialized_;
    target_value_ = other.target_value_;
    current_value_ = other.current_value_;
    increment_ = other.increment_;
    ramp_length_ = other.ramp_length_;
    scale_ = other.scale_;

    return *this;
}

void RampProcessor::SetTargetValue(const float value) {
    if (!initialized_) {
        current_value_ = scale_ * value;
        initialized_ = true;
    }
    target_value_ = scale_ * value;
    switch (ramp_mode_) {
        case ConstantRate:
            increment_ = target_value_ > current_value_ ? kDefaultRampIncrement : -kDefaultRampIncrement;
            ramp_length_ =
                static_cast<int>(std::abs(target_value_ - current_value_) * static_cast<float>(kDefaultUnitRampLength));
            break;
        case ConstantDuration:
            increment_ = (target_value_ - current_value_) / kDefaultTotalRampLength;
            ramp_length_ = kDefaultTotalRampLength;
            if (std::abs(increment_) < kDefaultMinimumRampIncrement) {
                increment_ = increment_ > 0 ? kDefaultMinimumRampIncrement : -kDefaultMinimumRampIncrement;
                ramp_length_ = static_cast<int>(std::abs(target_value_ - current_value_) /
                                                static_cast<float>(kDefaultMinimumRampIncrement));
            }
            break;
    }
}

int RampProcessor::GetRampValue(std::vector<float>& values, const int num_frames) {
    if (ramp_length_ > 0) {
        int process_length = std::min(ramp_length_, num_frames);
        for (size_t frame = 0; frame < process_length; ++frame) {
            values[frame] = current_value_;
            current_value_ = current_value_ + increment_;
        }
        ramp_length_ -= process_length;
        return process_length;
    } else {
        return 0;
    }
}

}  // namespace avs3renderer