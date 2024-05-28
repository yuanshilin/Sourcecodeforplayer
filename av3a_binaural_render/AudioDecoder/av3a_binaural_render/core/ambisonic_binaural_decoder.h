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

#ifndef AMBISONIC_BINAURAL_DECODER_H
#define AMBISONIC_BINAURAL_DECODER_H

#include "static_convolver.h"
#include "spherical_harmonic_hrir.h"
#include <memory>
#include <vector>

namespace avs3renderer {
class AmbisonicBinauralDecoder {
public:
    AmbisonicBinauralDecoder() = default;
    virtual ~AmbisonicBinauralDecoder() = default;

    /**
     * Decode ambisonic buffer to interleaved binaural output.
     * Usually, this API will accumulated decoded buffer to output address except for ResonanceAmbisonicBinauralDecoder
     *
     * @param input_buffer_ptr Planar Ambisonics buffer
     * @param num_frames
     * @param output_buffer_ptr Pointer to interleaved output address
     */
    virtual void Process(const float* input_buffer_ptr,
                         size_t num_frames,
                         float* output_buffer_ptr,
                         bool accumulative = true) = 0;

    /**
     * Decode ambisonic buffer to planar binaural output
     * Usually, this API will accumulated decoded buffer to output address except for ResonanceAmbisonicBinauralDecoder
     *
     * @param input_buffer_ptr Planar Ambisonics buffer
     * @param num_frames Number of frames to be decoded
     * @param output_buffer_ptr Pointers to planar output address, this address should have two channels
     */
    virtual void Process(const float* input_buffer_ptr,
                         size_t num_frames,
                         float* const* output_buffer_ptr,
                         bool accumulative = true) = 0;

    /**
     * Decode Planar ambisonic buffer to planar binaural output
     * Usually, this API will accumulated decoded buffer to output address except for ResonanceAmbisonicBinauralDecoder
     *
     * @param input_buffer_ptr Planar Ambisonics buffer
     * @param num_frames Number of frames to be decoded
     * @param output_buffer_ptr Pointers to planar output address, this address should have two channels
     */
    virtual void Process(const float* const* input_buffer_ptr,
                         size_t num_frames,
                         float* const* output_buffer_ptr,
                         bool accumulative = true) = 0;

    /**
     * for VST Ambisonic decode
     * @param input_buffer_ptr
     * @param num_frames
     * @param ambisonic_channel 输入的第几声道
     * @param output_buffer_ptr
     * @param output_channels 实际的输出声道数，由VST 设置
     */
    virtual void Process(const float* input_buffer_ptr,
                         size_t num_frames,
                         size_t ambisonic_channel,
                         float** output_buffer_ptr,
                         int output_channels) = 0;
};

class AmbisonicBinauralDecoderImpl : public AmbisonicBinauralDecoder {
public:
    AmbisonicBinauralDecoderImpl(size_t sample_rate, size_t frames_per_buffer, size_t ambisonic_order);

    void Process(const float* input_buffer_ptr,
                 size_t num_frames,
                 float* output_buffer_ptr,
                 bool accumulative) override;

    void Process(const float* input_buffer_ptr,
                 size_t num_frames,
                 float* const* output_buffer_ptr,
                 bool accumulative) override;

    void Process(const float* const* input_buffer_ptr,
                 size_t num_frames,
                 float* const* output_buffer_ptr,
                 bool accumulative) override;

    void Process(const float* input_buffer_ptr,
                 size_t num_frames,
                 size_t ambisonic_channel,
                 float** output_buffer_ptr,
                 int output_channels) override;

private:
    size_t ambisonic_order_;
    std::unique_ptr<SphericalHarmonicHrir> hrir_;
    std::vector<std::unique_ptr<StaticConvolver>> convolver_;
    std::vector<float> temp_buffer_;
};

}  // namespace avs3renderer
#endif  // AMBISONIC_BINAURAL_DECODER_H
