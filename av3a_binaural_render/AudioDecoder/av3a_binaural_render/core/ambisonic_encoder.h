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

#ifndef AMBISONIC_ENCODER_H
#define AMBISONIC_ENCODER_H

#include <vector>
#include <memory>
#include "definitions.h"
#include "core/ramp_processor.h"
#include "core/sound_source.h"

namespace avs3renderer {

class AmbisonicEncoder {
public:
    explicit AmbisonicEncoder(int order, int frames_per_buffer = 512);

    /**
     * Change ambisonic encoder order to |order|
     * This method is **NEITHER** thread-safe nor real-time-safe. Avoid calling while processing.
     *
     * @param order
     */
    void UpdateEncoderOrder(int order);

    /**
     * Encode a mono input buffer to ambisonic and accumulate to output buffer according to its position.
     * Result will |ACCUMULATE| to output buffer.
     * @param input_buffer Pointer to mono input buffer.
     * @param planar_output_buffer Pointer to ambisonic output buffer in planar format.
     * @param output_channels Channels of output buffer, usually it should equal to (n+1)^2. Processing will be
     * terminated when channel id > output_channels to avoid write out of range.
     * @param num_frames Number of frames of input buffer.
     * @param position Relative position to Listener (or a defined orientation) of input audio stream.
     * @param gain Gain applied to input buffer, this value will be ramped if changed.
     */
    void Process(const float* input_buffer,
                 float* const* planar_output_buffer,
                 int output_channels,
                 int num_frames,
                 const Vector3f& position,
                 float gain = 1.0f);

    /**
     * Encode a mono input buffer to ambisonic and accumulate to output buffer according to its position.
     * Result will |ACCUMULATE| to output buffer.
     * @param input_buffer Pointer to mono input buffer.
     * @param planar_output_buffer Pointer to ambisonic output buffer in planar format, all channels are stored
     * contiguously.
     * @param output_channels Channels of output buffer, usually it should equal to (n+1)^2. Processing will be
     * terminated when channel id > output_channels to avoid write out of range.
     * @param num_frames Number of frames of input buffer.
     * @param position Relative position to Listener (or a defined orientation) of input audio stream.
     * @param gain Gain applied to input buffer, this value will be ramped if changed.
     */
    void Process(const float* input_buffer,
                 float* planar_output_buffer,
                 int output_channels,
                 int num_frames,
                 const Vector3f& position,
                 float gain = 1.0f);

private:
    int ambisonic_order_;
    int ambisonic_channels_;
    int frames_per_buffer_;
    std::vector<float> encoder_weights_;
    std::vector<float> encoder_coeffs_;
    std::vector<RampProcessor> ambisonic_ramps_;

    // Doppler stuffs
    std::atomic_int sample_rate_;

};

}  // namespace avs3renderer

#endif  // AMBISONIC_ENCODER_H
