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

#include "spherical_harmonic_hrir.h"
#include "core/utils.h"
#include "ambisonics/spherical_harmonics_constants.h"
#include "core/logging.h"
#include "ext/hrtf_database/hrtf_assets.h"
#include "ext/libsamplerate/samplerate.h"
#include "ext/simd/simd_utils.h"
#include <sstream>

constexpr int kHrirSampleRate = 48000;

namespace avs3renderer {

void SadieSphericalHarmonicHrir::CreateShHrirsFromAssets(const std::string &filename,
                                                         size_t target_sample_rate_hz,
                                                         int order,
                                                         std::vector<std::vector<float, FloatAllocator>> *hrirs) {

    const int wav_sample_rate_hz = kHrirSampleRate;
    const int wav_channel = (order + 1) *  (order + 1);

    if (avs3renderer::HrtfAssets::kAssetMap.find(filename) == avs3renderer::HrtfAssets::kAssetMap.end()) {
        LogE("HRIR is nullptr");
        exit(-1);
    }

    auto& hrir_wav = avs3renderer::HrtfAssets::kAssetMap.at(filename);
    const int wav_length = (int)hrir_wav[0].size();

    std::vector<std::vector<float, FloatAllocator>> float_planar_samples(
            wav_channel, std::vector<float, FloatAllocator>(wav_length));

    for (int c = 0; c < wav_channel; ++c) {
        for (int f = 0; f < wav_length; ++f) {
            float_planar_samples[c][f] = hrir_wav[c][f];
        }
    }

    if (wav_sample_rate_hz == target_sample_rate_hz) {
        (*hrirs) = float_planar_samples;
    } else {
        const double src_ratio = static_cast<double>(target_sample_rate_hz) / static_cast<double>(wav_sample_rate_hz);
        const auto expected_out_length = static_cast<size_t>(std::floor(static_cast<double>(wav_length) * src_ratio));

        hrirs->resize(wav_channel);
        for (auto &hrir : *hrirs) {
            hrir.resize(expected_out_length);
        }

        std::vector<SRC_DATA> src_data(wav_channel);

        for (int c = 0; c < wav_channel; ++c) {
            src_data[c].data_in = float_planar_samples[c].data();
            src_data[c].input_frames = wav_length;
            src_data[c].data_out = hrirs->at(c).data();
            src_data[c].output_frames = expected_out_length;
            src_data[c].src_ratio = src_ratio;
            src_simple(&src_data[c], SRC_SINC_BEST_QUALITY, 1);
        }
    }
}

SadieSphericalHarmonicHrir::SadieSphericalHarmonicHrir(size_t order, size_t sample_rate)
        : SphericalHarmonicHrir(), sample_rate_(sample_rate) {
    static std::vector<std::string> sh_hrir_filenames = {"Order1.hrir",
                                                         "Order2.hrir",
                                                         "Order3.hrir",
                                                         "Order4.hrir",
                                                         "Order5.hrir",
                                                         "Order6.hrir",
                                                         "Order7.hrir"};

    std::string sh_hrir_filename = sh_hrir_filenames[order - 1];
    CreateShHrirsFromAssets(sh_hrir_filename, sample_rate_, (int)order, &hrir_data_);
    num_channels_ = hrir_data_.size();
    num_samples_ = hrir_data_[0].size();
}

const std::vector<float, FloatAllocator> &SadieSphericalHarmonicHrir::GetHrirOfChannel(size_t channel) const {
    return hrir_data_.at(channel);
}
}  // namespace avs3renderer
