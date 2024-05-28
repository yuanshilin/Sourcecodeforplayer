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

#include "static_convolver.h"
#include "utils.h"
#include "logging.h"
#include "ext/simd/simd_utils.h"

namespace avs3renderer {

StaticConvolver::StaticConvolver(size_t frames_per_buffer, size_t ir_length)
    : StaticConvolver(frames_per_buffer, ir_length, std::make_shared<FftManager>(frames_per_buffer)) {
}

StaticConvolver::StaticConvolver(size_t frames_per_buffer, size_t ir_length, std::shared_ptr<FftManager> fft_manager)
    : frames_per_buffer_(frames_per_buffer),
      ir_length_(CeilToMultipleOfFramesPerBuffer(ir_length, frames_per_buffer)),
      num_partitions_(ir_length_ / frames_per_buffer),
      fft_manager_(fft_manager),
      fft_size_(fft_manager->GetFftSize()),
      freq_domain_kernel_(num_partitions_, std::vector<float, FloatAllocator>(fft_size_)),
      filtered_time_domain_buffer_(2, std::vector<float, FloatAllocator>(fft_size_)),
      freq_domain_input_(num_partitions_, std::vector<float, FloatAllocator>(fft_size_)),
      freq_domain_accumulator_(fft_size_),
      time_domain_temp_buffer_(frames_per_buffer_),
      buffer_selector_(0),
      curr_front_buffer_(0) {
}

void StaticConvolver::SetKernel(const float* ir, size_t ir_length) {
    size_t new_ir_length = CeilToMultipleOfFramesPerBuffer(ir_length, frames_per_buffer_);
    size_t new_num_partitions = new_ir_length / frames_per_buffer_;
    if (new_num_partitions != num_partitions_) {
        freq_domain_kernel_.resize(new_num_partitions);
        if (new_num_partitions > num_partitions_) {
            size_t insert_n = new_num_partitions - num_partitions_;
            for (int n = 0; n < insert_n; ++n) {
                freq_domain_input_.insert(freq_domain_input_.begin() + static_cast<int>(curr_front_buffer_),
                                          std::vector<float, FloatAllocator>(fft_size_));
                curr_front_buffer_ = (curr_front_buffer_ + 1) % num_partitions_;
            }
        } else {
            size_t remove_n = num_partitions_ - new_num_partitions;
            auto erase_itr = freq_domain_input_.begin() + static_cast<int>(curr_front_buffer_);
            for (int n = 0; n < remove_n; ++n) {
                if (erase_itr == freq_domain_input_.begin()) {
                    erase_itr = freq_domain_input_.end();
                }
                --erase_itr;
                freq_domain_input_.erase(erase_itr);
            }
            curr_front_buffer_ = curr_front_buffer_ > remove_n ? curr_front_buffer_ - remove_n : 0;
        }
    }

    // Compute freq_domain_kernel_ by new IR kernel.
    int copied_samples = 0;
    for (int p = 0; p < new_num_partitions; ++p) {
        int num_frames_to_copy = std::min(frames_per_buffer_, ir_length - copied_samples);
        std::copy_n(ir + frames_per_buffer_ * p, num_frames_to_copy, time_domain_temp_buffer_.data());
        std::fill(time_domain_temp_buffer_.begin() + num_frames_to_copy, time_domain_temp_buffer_.end(), 0.f);
        fft_manager_->FreqFromTimeDomain(time_domain_temp_buffer_, &freq_domain_kernel_[p]);
        copied_samples += num_frames_to_copy;
    }
    num_partitions_ = new_num_partitions;
    ir_length_ = new_ir_length;
}

void StaticConvolver::Process(const float* input_buffer, float* output_buffer) {
    curr_front_buffer_ = (curr_front_buffer_ + num_partitions_ - 1) % num_partitions_;
    buffer_selector_ = !buffer_selector_;

    std::fill(freq_domain_accumulator_.begin(), freq_domain_accumulator_.end(), 0.f);
    fft_manager_->FreqFromTimeDomain(input_buffer, freq_domain_input_[curr_front_buffer_].data(), frames_per_buffer_);
    for (int p = 0; p < num_partitions_; ++p) {
        size_t input_idx = (curr_front_buffer_ + p) % num_partitions_;
        fft_manager_->FreqDomainConvolution(freq_domain_input_[input_idx], freq_domain_kernel_[p],
                                            &freq_domain_accumulator_);
    }

    fft_manager_->TimeFromFreqDomain(freq_domain_accumulator_, &filtered_time_domain_buffer_[buffer_selector_]);
    if (frames_per_buffer_ == fft_size_ / 2) {
        vraudio_simd::AddPointwise(frames_per_buffer_, filtered_time_domain_buffer_[buffer_selector_].data(),
                                   filtered_time_domain_buffer_[!buffer_selector_].data() + frames_per_buffer_,
                                   output_buffer);
    } else {
        for (int f = 0; f < frames_per_buffer_; ++f) {
            output_buffer[f] = filtered_time_domain_buffer_[buffer_selector_][f] +
                               filtered_time_domain_buffer_[!buffer_selector_][f + frames_per_buffer_];
        }
    }
}

}  // namespace avs3renderer
