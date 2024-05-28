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

#ifndef SPHERICAL_HARMONIC_HRIR_H
#define SPHERICAL_HARMONIC_HRIR_H

#include <vector>
#include <string>
#include "core/aligned_allocator.h"

namespace avs3renderer {
/**
 * Manager of spherical harmonic HRIR
 */
class SphericalHarmonicHrir {
public:
    SphericalHarmonicHrir() = default;
    virtual ~SphericalHarmonicHrir() = default;
    virtual const std::vector<float, FloatAllocator>& GetHrirOfChannel(size_t channel) const = 0;
    virtual size_t GetNumSamples() const = 0;
    virtual size_t GetNumChannels() const = 0;
};

class SadieSphericalHarmonicHrir : public SphericalHarmonicHrir {
public:
    SadieSphericalHarmonicHrir(size_t order, size_t sample_rate);
    const std::vector<float, FloatAllocator>& GetHrirOfChannel(size_t channel) const override;
    size_t GetNumSamples() const override {
        return num_samples_;
    }
    size_t GetNumChannels() const override {
        return num_channels_;
    }

private:
    static void CreateShHrirsFromAssets(const std::string& filename,
                                        size_t target_sample_rate_hz,
                                        int order,
                                        std::vector<std::vector<float, FloatAllocator>>* hrirs);
    size_t num_samples_;
    size_t num_channels_;
    size_t sample_rate_;
    std::vector<std::vector<float, FloatAllocator>> hrir_data_;
};
}  // namespace avs3renderer

#endif  // SPHERICAL_HARMONIC_HRIR_H
