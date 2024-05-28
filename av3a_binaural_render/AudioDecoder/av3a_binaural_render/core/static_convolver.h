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

#ifndef STATIC_CONVOLVER_H
#define STATIC_CONVOLVER_H

#include <cstddef>
#include <memory>
#include "core/fft_manager.h"
#include "core/aligned_allocator.h"

namespace avs3renderer {
class StaticConvolver {
public:
    StaticConvolver(size_t frames_per_buffer, size_t ir_length);
    StaticConvolver(size_t frames_per_buffer, size_t ir_length, std::shared_ptr<FftManager> fft_manager);
    void SetKernel(const float* ir, size_t ir_length);
    /**
     * FFT Convolve input_buffer with kernel.
     * This method will OVERWRITE output_buffer.
     * @param input_buffer
     * @param output_buffer
     */
    void Process(const float* input_buffer, float* output_buffer);

private:
    size_t frames_per_buffer_;
    size_t ir_length_;
    size_t num_partitions_;
    size_t fft_size_;
    size_t buffer_selector_;
    size_t curr_front_buffer_;
    std::vector<std::vector<float, FloatAllocator> > freq_domain_kernel_;
    std::vector<std::vector<float, FloatAllocator> > filtered_time_domain_buffer_;
    std::vector<std::vector<float, FloatAllocator> > freq_domain_input_;
    std::vector<float, FloatAllocator> freq_domain_accumulator_;
    std::vector<float, FloatAllocator> time_domain_temp_buffer_;
    std::shared_ptr<FftManager> fft_manager_;
};
}  // namespace avs3renderer

#endif  // STATIC_CONVOLVER_H
