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

#include "ambisonics/spherical_harmonics.h"
#include "ambisonic_encoder.h"
#include "core/utils.h"

namespace avs3renderer {

AmbisonicEncoder::AmbisonicEncoder(int order, int frames_per_buffer)
    : ambisonic_order_(order),
      ambisonic_channels_((order + 1) * (order + 1)),
      frames_per_buffer_(frames_per_buffer),
      encoder_coeffs_((order + 1) * (order + 1), 0.0f),
      encoder_weights_(frames_per_buffer, 0.0f),
      sample_rate_(0) {
    for (int i = 0; i < ambisonic_channels_; ++i) {
        ambisonic_ramps_.emplace_back(RampProcessor::ConstantRate);
    }
}

void AmbisonicEncoder::UpdateEncoderOrder(int order) {
    ambisonic_order_ = order;
    ambisonic_channels_ = (order + 1) * (order + 1);
    encoder_coeffs_.resize(ambisonic_channels_, 0.0f);
    ambisonic_ramps_.clear();
    for (int i = 0; i < ambisonic_channels_; ++i) {
        ambisonic_ramps_.emplace_back(RampProcessor::ConstantRate);
    }
}


void AmbisonicEncoder::Process(const float* input_buffer,
                               float* const* planar_output_buffer,
                               int output_channels,
                               int num_frames,
                               const Vector3f& position,
                               float gain) {
    if (num_frames != encoder_weights_.size()) {
        encoder_weights_.resize(num_frames, 0.0f);
    }

    Vector3f relative_dist = AudioPositionFromWorldPosition(position.normalized());
    RealSphericalHarmonics(ambisonic_order_, relative_dist.x(), relative_dist.y(), relative_dist.z(),
                           encoder_coeffs_.data());
    for (int channel = 0; (channel < ambisonic_channels_ && channel < output_channels); ++channel) {
        // Ramp for distance attenuation changing
        float inverse_distance = 1 / (position.norm() < 1 ? 1 : position.norm());
        ambisonic_ramps_.at(channel).SetScale(gain * inverse_distance);
        // Ramp for direction changing
        ambisonic_ramps_.at(channel).SetTargetValue(encoder_coeffs_[channel]);
        int ramp_length = ambisonic_ramps_.at(channel).GetRampValue(encoder_weights_, num_frames);

        if (ramp_length > 0) {
            vraudio_simd::MultiplyAndAccumulatePointwise(
                ramp_length, encoder_weights_.data(),
                input_buffer, planar_output_buffer[channel]);
            vraudio_simd::ScalarMultiplyAndAccumulate(
                num_frames - ramp_length, ambisonic_ramps_.at(channel).target_value(),
                input_buffer + ramp_length,
                planar_output_buffer[channel] + ramp_length);
        } else {
            vraudio_simd::ScalarMultiplyAndAccumulate(
                num_frames, ambisonic_ramps_.at(channel).target_value(),
                input_buffer, planar_output_buffer[channel]);
        }
    }
}

void AmbisonicEncoder::Process(const float* input_buffer,
                               float* planar_output_buffer,
                               int output_channels,
                               int num_frames,
                               const Vector3f& position,
                               float gain) {
    if (num_frames != encoder_weights_.size()) {
        encoder_weights_.resize(num_frames, 0.0f);
    }

    Vector3f relative_dist = AudioPositionFromWorldPosition(position.normalized());
    RealSphericalHarmonics(ambisonic_order_, relative_dist.x(), relative_dist.y(), relative_dist.z(),
                           encoder_coeffs_.data());
    for (int channel = 0; (channel < ambisonic_channels_ && channel < output_channels); ++channel) {
        // Ramp for distance attenuation changing
        float inverse_distance = 1 / position.norm();
        inverse_distance = inverse_distance > 1 ? 1 : inverse_distance;
        ambisonic_ramps_.at(channel).SetScale(gain * inverse_distance);
        // Ramp for direction changing
        ambisonic_ramps_.at(channel).SetTargetValue(encoder_coeffs_[channel]);
        int ramp_length = ambisonic_ramps_.at(channel).GetRampValue(encoder_weights_, num_frames);

        if (ramp_length > 0) {
            vraudio_simd::MultiplyAndAccumulatePointwise(
                ramp_length, encoder_weights_.data(),
                input_buffer,
                planar_output_buffer + num_frames * channel);
            vraudio_simd::ScalarMultiplyAndAccumulate(
                num_frames - ramp_length, ambisonic_ramps_.at(channel).target_value(),
                input_buffer + ramp_length,
                planar_output_buffer + num_frames * channel + ramp_length);
        } else {
            vraudio_simd::ScalarMultiplyAndAccumulate(
                num_frames, ambisonic_ramps_.at(channel).target_value(),
                input_buffer,
                planar_output_buffer + num_frames * channel);
        }
    }
}

}  // namespace avs3renderer
