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

#include "ambisonic_binaural_decoder.h"
#include "ambisonics/spherical_harmonics.h"
#include "core/fft_manager.h"

namespace avs3renderer {
AmbisonicBinauralDecoderImpl::AmbisonicBinauralDecoderImpl(size_t sample_rate,
                                                           size_t frames_per_buffer,
                                                           size_t ambisonic_order)
    : AmbisonicBinauralDecoder(), ambisonic_order_(ambisonic_order), temp_buffer_(frames_per_buffer) {
    hrir_ = std::unique_ptr<SphericalHarmonicHrir>(
        new SadieSphericalHarmonicHrir(ambisonic_order, sample_rate));
    convolver_.reserve(hrir_->GetNumChannels());

    auto fft_manager = std::make_shared<FftManager>(frames_per_buffer);
    for (int c = 0; c < hrir_->GetNumChannels(); ++c) {
        convolver_.emplace_back(new StaticConvolver(frames_per_buffer, hrir_->GetNumSamples(), fft_manager));
        convolver_[c]->SetKernel(hrir_->GetHrirOfChannel(c).data(), hrir_->GetNumSamples());
    }
}

void AmbisonicBinauralDecoderImpl::Process(const float* input_buffer_ptr,
                                           size_t num_frames,
                                           float* output_buffer_ptr,
                                           bool accumulative) {
    if(!accumulative)
        std::fill_n(output_buffer_ptr, num_frames * 2, 0.0f);
    for (int channel = 0; channel < kNumAmbisonicChannels[ambisonic_order_]; ++channel) {
        convolver_[channel]->Process(input_buffer_ptr + channel * num_frames, temp_buffer_.data());
        int degree = DegreeFromAcnSequence(channel);
        if (degree < 0) {
            for (int f = 0; f < num_frames; ++f) {
                output_buffer_ptr[2 * f] += temp_buffer_[f];
                output_buffer_ptr[2 * f + 1] -= temp_buffer_[f];
            }
        } else {
            for (int f = 0; f < num_frames; ++f) {
                output_buffer_ptr[2 * f] += temp_buffer_[f];
                output_buffer_ptr[2 * f + 1] += temp_buffer_[f];
            }
        }
    }
}

void AmbisonicBinauralDecoderImpl::Process(const float* input_buffer_ptr,
                                           const size_t num_frames,
                                           float* const* output_buffer_ptr,
                                           bool accumulative) {
    if(!accumulative) {
        std::fill_n(output_buffer_ptr[0], num_frames, 0.0f);
        std::fill_n(output_buffer_ptr[1], num_frames, 0.0f);
    }

    for (int channel = 0; channel < kNumAmbisonicChannels[ambisonic_order_]; ++channel) {
        convolver_[channel]->Process(input_buffer_ptr + channel * num_frames, temp_buffer_.data());
        int degree = DegreeFromAcnSequence(channel);
        if (degree < 0) {
            for (int f = 0; f < num_frames; ++f) {
                output_buffer_ptr[0][f] += temp_buffer_[f];
                output_buffer_ptr[1][f] -= temp_buffer_[f];
            }
        } else {
            for (int f = 0; f < num_frames; ++f) {
                output_buffer_ptr[0][f] += temp_buffer_[f];
                output_buffer_ptr[1][f] += temp_buffer_[f];
            }
        }
    }
}

void AmbisonicBinauralDecoderImpl::Process(const float* const* input_buffer_ptr,
                                           size_t num_frames,
                                           float* const* output_buffer_ptr,
                                           bool accumulative) {
    if (!accumulative) {
        std::fill_n(output_buffer_ptr[0], num_frames, 0.0f);
        std::fill_n(output_buffer_ptr[1], num_frames, 0.0f);
    }

    for (int channel = 0; channel < kNumAmbisonicChannels[ambisonic_order_]; ++channel) {
        convolver_[channel]->Process(input_buffer_ptr[channel], temp_buffer_.data());
        int degree = DegreeFromAcnSequence(channel);
        if (degree < 0) {
            for (int f = 0; f < num_frames; ++f) {
                output_buffer_ptr[0][f] += temp_buffer_[f];
                output_buffer_ptr[1][f] -= temp_buffer_[f];
            }
        } else {
            for (int f = 0; f < num_frames; ++f) {
                output_buffer_ptr[0][f] += temp_buffer_[f];
                output_buffer_ptr[1][f] += temp_buffer_[f];
            }
        }
    }
}

void AmbisonicBinauralDecoderImpl::Process(const float* input_buffer_ptr,
                                           const size_t num_frames,
                                           const size_t ambisonic_channel,
                                           float** output_buffer_ptr,
                                           const int output_channels) {
    convolver_[ambisonic_channel]->Process(input_buffer_ptr, temp_buffer_.data());
    int degree = DegreeFromAcnSequence(ambisonic_channel);
    if (output_channels < 2) {
        if (degree < 0) {
            for (int i = 0; i < num_frames; ++i) {
                output_buffer_ptr[0][i] += temp_buffer_[i];
            }
        } else {
            for (int i = 0; i < num_frames; ++i) {
                output_buffer_ptr[0][i] += temp_buffer_[i];
            }
        }
    } else {
        if (degree < 0) {
            for (int i = 0; i < num_frames; ++i) {
                output_buffer_ptr[0][i] += temp_buffer_[i];
                output_buffer_ptr[1][i] -= temp_buffer_[i];
            }
        } else {
            for (int i = 0; i < num_frames; ++i) {
                output_buffer_ptr[0][i] += temp_buffer_[i];
                output_buffer_ptr[1][i] += temp_buffer_[i];
            }
        }
    }
}

}  // namespace avs3renderer
