/*
Copyright 2018 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "fft_manager.h"
#include "utils.h"
#include "ext/simd/simd_utils.h"
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <functional>

namespace avs3renderer {

namespace {

// for |fft_size_|s less than 2^14, the stack is used, on the reccomendation of
// the author of the pffft library.
const size_t kPffftMaxStackSize = 16384;

const size_t kNumMonoChannels = 1;

}  // namespace

// The pffft implementation requires a minimum fft size of 32 samples.
const size_t FftManager::kMinFftSize = 32;

FftManager::FftManager(size_t frames_per_buffer)
    : fft_size_(std::max(NextPowTwo(frames_per_buffer) * 2, kMinFftSize)),
      frames_per_buffer_(frames_per_buffer),
      inverse_fft_scale_(1.0f / static_cast<float>(fft_size_)),
      temp_zeropad_buffer_(fft_size_),
      temp_freq_buffer_(fft_size_) {
    // Suggested pffft initialization.
    if (fft_size_ > kPffftMaxStackSize) {
        // Allocate memory for work space factors etc, Size reccomended by pffft.
        const size_t num_bytes = 2 * fft_size_ * sizeof(float);
        pffft_workspace_ = reinterpret_cast<float*>(pffft_aligned_malloc(num_bytes));
    }

    fft_ = pffft_new_setup(static_cast<int>(fft_size_), PFFFT_REAL);

    std::fill(temp_zeropad_buffer_.begin(), temp_zeropad_buffer_.end(), 0.f);
}

FftManager::~FftManager() {
    pffft_destroy_setup(fft_);
    if (pffft_workspace_ != nullptr) {
        pffft_aligned_free(pffft_workspace_);
    }
}

void FftManager::FreqFromTimeDomain(const SamplesBuffer& time_channel, SamplesBuffer* freq_channel) {
    // Perform forward FFT transform.
    if (time_channel.size() == fft_size_) {
        pffft_transform(fft_, time_channel.data(), freq_channel->data(), pffft_workspace_, PFFFT_FORWARD);
    } else {
        std::copy_n(time_channel.data(), frames_per_buffer_, temp_zeropad_buffer_.data());
        pffft_transform(fft_, temp_zeropad_buffer_.data(), freq_channel->data(), pffft_workspace_, PFFFT_FORWARD);
    }
}

void FftManager::FreqFromTimeDomain(const float* time_buffer, float* freq_buffer, int buffer_size) {
    // Perform forward FFT transform.
    if (buffer_size == fft_size_) {
        pffft_transform(fft_, time_buffer, freq_buffer, pffft_workspace_, PFFFT_FORWARD);
    } else {
        std::copy_n(time_buffer, frames_per_buffer_, temp_zeropad_buffer_.data());
        pffft_transform(fft_, temp_zeropad_buffer_.data(), freq_buffer, pffft_workspace_, PFFFT_FORWARD);
    }
}

void FftManager::TimeFromFreqDomain(const SamplesBuffer& freq_channel, SamplesBuffer* time_channel) {
    // Perform reverse FFT transform.
    const size_t time_channel_size = time_channel->size();
    if (time_channel_size == fft_size_) {
        pffft_transform(fft_, freq_channel.data(), time_channel->data(), pffft_workspace_, PFFFT_BACKWARD);
    } else {
        auto& temp_channel = temp_freq_buffer_;
        pffft_transform(fft_, freq_channel.data(), temp_channel.data(), pffft_workspace_, PFFFT_BACKWARD);
        std::copy_n(temp_channel.data(), frames_per_buffer_, time_channel->data());
    }
}

void FftManager::TimeFromFreqDomain(const float* freq_buffer, float* time_buffer, int buffer_size) {
    // Perform reverse FFT transform.
    if (buffer_size == fft_size_) {
        pffft_transform(fft_, freq_buffer, time_buffer, pffft_workspace_, PFFFT_BACKWARD);
    } else {
        auto& temp_channel = temp_freq_buffer_;
        pffft_transform(fft_, freq_buffer, temp_channel.data(), pffft_workspace_, PFFFT_BACKWARD);
        std::copy_n(temp_channel.data(), frames_per_buffer_, time_buffer);
    }
}

void FftManager::ApplyReverseFftScaling(SamplesBuffer* time_channel) {
    // Normalization must be performed here as we normally do this as part of the
    // convolution.
    std::transform(time_channel->begin(), time_channel->end(), time_channel->begin(),
                   std::bind(std::multiplies<float>(), std::placeholders::_1, inverse_fft_scale_));
}

void FftManager::GetCanonicalFormatFreqBuffer(const SamplesBuffer& input, SamplesBuffer* output) {
    pffft_zreorder(fft_, input.data(), output->data(), PFFFT_FORWARD);
}

void FftManager::GetCanonicalFormatFreqBuffer(const float* input, float* output) {
    pffft_zreorder(fft_, input, output, PFFFT_FORWARD);
}

void FftManager::GetPffftFormatFreqBuffer(const SamplesBuffer& input, SamplesBuffer* output) {
    pffft_zreorder(fft_, input.data(), output->data(), PFFFT_BACKWARD);
}

void FftManager::GetPffftFormatFreqBuffer(const float* input, float* output) {
    pffft_zreorder(fft_, input, output, PFFFT_BACKWARD);
}

void FftManager::MagnitudeFromCanonicalFreqBuffer(const SamplesBuffer& freq_channel, SamplesBuffer* magnitude_channel) {
    (*magnitude_channel)[0] = std::abs(freq_channel[0]);
    vraudio_simd::ApproxComplexMagnitude(frames_per_buffer_ - 1, freq_channel.data() + 2, magnitude_channel->data() + 1);
    (*magnitude_channel)[frames_per_buffer_] = std::abs(freq_channel[1]);
}

void FftManager::CanonicalFreqBufferFromMagnitudeAndPhase(const SamplesBuffer& magnitude_channel,
                                                          const SamplesBuffer& phase_channel,
                                                          SamplesBuffer* canonical_freq_channel) {
    (*canonical_freq_channel)[0] = magnitude_channel[0];
    (*canonical_freq_channel)[1] = -magnitude_channel[frames_per_buffer_];
    for (size_t i = 1, j = 2; i < frames_per_buffer_; ++i, j += 2) {
        (*canonical_freq_channel)[j] = magnitude_channel[i] * std::cos(phase_channel[i]);
        (*canonical_freq_channel)[j + 1] = magnitude_channel[i] * std::sin(phase_channel[i]);
    }
}

void FftManager::CanonicalFreqBufferFromMagnitudeAndSinCosPhase(size_t phase_offset,
                                                                const SamplesBuffer& magnitude_channel,
                                                                const SamplesBuffer& sin_phase_channel,
                                                                const SamplesBuffer& cos_phase_channel,
                                                                SamplesBuffer* canonical_freq_channel) {
    static const size_t kSimdLength = 4;

    (*canonical_freq_channel)[0] = magnitude_channel[0];
    (*canonical_freq_channel)[1] = -magnitude_channel[frames_per_buffer_];
    // Continue on till we can gaurantee alignment in our audio buffer.
    for (size_t i = 1, j = 2; i <= kSimdLength; ++i, j += 2) {
        (*canonical_freq_channel)[j] = magnitude_channel[i] * cos_phase_channel[i + phase_offset];
        (*canonical_freq_channel)[j + 1] = magnitude_channel[i] * sin_phase_channel[i + phase_offset];
    }
    vraudio_simd::ComplexInterleavedFormatFromMagnitudeAndSinCosPhase(
        2 * (frames_per_buffer_ - kSimdLength), &magnitude_channel[kSimdLength],
        &cos_phase_channel[kSimdLength + phase_offset], &sin_phase_channel[kSimdLength + phase_offset],
        &(*canonical_freq_channel)[2 * kSimdLength]);
}

void FftManager::FreqDomainConvolution(const SamplesBuffer& input_a,
                                       const SamplesBuffer& input_b,
                                       SamplesBuffer* scaled_output) {
    FreqDomainConvolution(input_a.data(), input_b.data(), scaled_output->data());
}

void FftManager::FreqDomainConvolution(const float* input_a, const float* input_b, float* scaled_output) {
    pffft_zconvolve_accumulate(fft_, input_a, input_b, scaled_output, inverse_fft_scale_);
}

}  // namespace avs3renderer