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

#include <vector>
#include <thread>
#include <string>
#include <unordered_map>

#include "avs3_audio.h"
#include "ambisonics/spherical_harmonics.h"
#include "core/listener.h"
#include "core/sound_source.h"
#include "core/ambisonic_binaural_decoder.h"
#include "core/aligned_allocator.h"
#include "Eigen/Dense"
#include "core/ambisonic_encoder.h"
#include "core/ambisonic_rotator.h"
#include "core/utils.h"
#include "core/lock_free_thread_safe_object_bank.h"

std::string version_string;
using SourceId = int;

struct listener_params {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    avs3renderer::Vector3f listener_position = {0.f, 0.f, 0.f};
    avs3renderer::Vector3f listener_front = {0.f, 0.f, 1.0f};
    avs3renderer::Vector3f listener_up = {0.f, 1.f, 0.f};
};

struct audio_context;

struct audio_processor {
    float source_gain = 1.0;
    source_mode source_mode_;
    std::shared_ptr<avs3renderer::AmbisonicEncoder> direct_sound_encoder_;

    audio_processor() {
    }

    audio_processor(::source_mode mode, audio_context *ctx);
};

struct audio_context {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    size_t sample_rate;
    size_t frames_per_buffer;
    size_t ambisonic_order;
    size_t max_source_count;
    size_t num_ambisonic_channels;
    std::shared_ptr<avs3renderer::Listener> listener;

    std::unordered_map<SourceId, std::shared_ptr<avs3renderer::SoundSource>> sound_sources_;
    std::shared_ptr<avs3renderer::LockFreeThreadSafeObjectBank<audio_processor>> audio_processors_;

    listener_params listener_params_;

    std::shared_ptr<avs3renderer::AmbisonicRotator> ambisonic_rotator_;
    std::shared_ptr<avs3renderer::AmbisonicBinauralDecoder> ambisonic_binaural_decoder_;

    Eigen::Matrix4f ambisonic_orientation = Eigen::Matrix4f::Identity();
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mapping_matrix_;  ///< Matrix storing mapping coefficients of matrix input

    bool has_ambisonic_input = false;

    // temp buffer
    std::vector<float, avs3renderer::FloatAllocator> temp_ambisonic_buffer_before_rotation;  ///< Planar temp buffer for storing submitted ambisonic
    ///< buffer. This buffer is needed because ambisonic
    ///< buffer is submitted by channel sometimes.
    std::vector<float, avs3renderer::FloatAllocator> temp_ambisonic_buffer;  ///< Planar temp buffer for storing sum of ambisonic encoding of all sources.
    std::vector<float, avs3renderer::FloatAllocator> temp_binaural_buffer;  ///< Interleaved temp buffer for storing sum of late reverb result of all sources.
};

audio_processor::audio_processor(::source_mode mode,
                                 audio_context *ctx) {
    source_mode_ = mode;
    switch (mode) {
        case SOURCE_BYPASS:
            break;
        case SOURCE_SPATIALIZE: {
            direct_sound_encoder_ =
                    std::make_shared<avs3renderer::AmbisonicEncoder>(ctx->ambisonic_order, ctx->frames_per_buffer);
            break;
        }
    }
}

const char *get_version(int *major, int *minor, int *patch) {
    if (major != nullptr)
        *major = MAJOR_VERSION;
    if (minor != nullptr)
        *minor = MINOR_VERSION;
    if (patch != nullptr)
        *patch = PATCH_VERSION;
    version_string =
            "\n"
            "AVS3 Binaural Renderer \n"
            "VERSION: " +
            std::to_string(MAJOR_VERSION) + "." + std::to_string(MINOR_VERSION) + "." +
            std::to_string(PATCH_VERSION);
    return version_string.c_str();
}

result audio_create_context(audio_context **ctx,
                            rendering_mode mode,
                            size_t frames_per_buffer,
                            size_t sample_rate) {
    *ctx = new audio_context;
    (*ctx)->frames_per_buffer = frames_per_buffer;
    (*ctx)->sample_rate = sample_rate;
    (*ctx)->max_source_count = 256;
    switch (mode) {
        case AMBISONIC_FIRST_ORDER: {
            (*ctx)->ambisonic_order = 1;
            break;
        }
        case AMBISONIC_SECOND_ORDER: {
            (*ctx)->ambisonic_order = 2;
            break;
        }
        case AMBISONIC_THIRD_ORDER: {
            (*ctx)->ambisonic_order = 3;
            break;
        }
        case AMBISONIC_FOURTH_ORDER: {
            (*ctx)->ambisonic_order = 4;
            break;
        }
        case AMBISONIC_FIFTH_ORDER: {
            (*ctx)->ambisonic_order = 5;
            break;
        }
        case AMBISONIC_SIXTH_ORDER: {
            (*ctx)->ambisonic_order = 6;
            break;
        }
        case AMBISONIC_SEVENTH_ORDER: {
            (*ctx)->ambisonic_order = 7;
            break;
        }
        default: {
            (*ctx)->ambisonic_order = 1;
            break;
        }
    }
    (*ctx)->num_ambisonic_channels = ((*ctx)->ambisonic_order + 1) * ((*ctx)->ambisonic_order + 1);
    (*ctx)->ambisonic_rotator_ = std::make_shared<avs3renderer::AmbisonicRotator>((*ctx)->ambisonic_order);
    (*ctx)->ambisonic_binaural_decoder_ = std::make_shared<avs3renderer::AmbisonicBinauralDecoderImpl>(
            sample_rate, frames_per_buffer, (*ctx)->ambisonic_order);
    (*ctx)->audio_processors_ =
            std::make_shared<avs3renderer::LockFreeThreadSafeObjectBank<audio_processor>>((*ctx)->max_source_count);

    (*ctx)->temp_binaural_buffer = std::vector<float, avs3renderer::FloatAllocator>(2 * frames_per_buffer);
    (*ctx)->temp_ambisonic_buffer_before_rotation =
            std::vector<float, avs3renderer::FloatAllocator>((*ctx)->num_ambisonic_channels * frames_per_buffer);
    (*ctx)->temp_ambisonic_buffer =
            std::vector<float, avs3renderer::FloatAllocator>((*ctx)->num_ambisonic_channels * frames_per_buffer);
    return SUCCESS;
}

// Initialize ray tracer / listener / sampler / integrator
result audio_initialize_context(audio_context *ctx) {
    if (ctx->listener != nullptr)
        return CONTEXT_REPEATED_INITIALIZATION;
    ctx->listener = std::shared_ptr<avs3renderer::OmniListener>(
            new avs3renderer::OmniListener(ctx->listener_params_.listener_position, ctx->listener_params_.listener_front,
                                           ctx->listener_params_.listener_up));
    return SUCCESS;
}

result audio_update_scene(audio_context *ctx) {
    return SUCCESS;
}

result audio_submit_mesh(audio_context *ctx,
                                   const float *vertices,
                                   int vertices_count,
                                   const int *indices,
                                   int indices_count,
                                   int material,
                                   int *geometry_id) {
    return SUCCESS;
}

result audio_submit_mesh_and_material_factor(audio_context *ctx,
                                             const float *vertices,
                                             int vertices_count,
                                             const int *indices,
                                             int indices_count,
                                             const float *absorption_factor,
                                             float scattering_factor,
                                             int *geometry_id) {

    return SUCCESS;
}

result audio_add_source(audio_context *ctx,
                        source_mode source_mode_,
                        const float *position,
                        int *source_id,
                        bool is_async) {
    const float front[3] = {0, 0, 1};
    const float up[3] = {0, 1, 0};
    return audio_add_source_with_orientation(ctx, source_mode_, position, front, up, source_id, is_async);
}

result audio_add_source_with_orientation(audio_context *ctx,
                                         source_mode mode,
                                         const float *position,
                                         const float *front,
                                         const float *up,
                                         int *source_id,
                                         bool is_async) {
    if (ctx == nullptr)
        return CONTEXT_NOT_CREATED;
    if (ctx->listener == nullptr)
        return CONTEXT_NOT_READY;
    if (ctx->audio_processors_->isFull())
        return ERROR;

    avs3renderer::Vector3f source_position(position[0], position[1], position[2]);
    avs3renderer::Vector3f source_front(front[0], front[1], front[2]);
    avs3renderer::Vector3f source_up(up[0], up[1], up[2]);

    std::shared_ptr<avs3renderer::SoundSource> source = std::make_shared<avs3renderer::OmniSoundSource>(
                                            source_position, source_front, source_up,
                                            ctx->audio_processors_->GetNextFreeEntryId());
    auto id = source->id();
    /// write api output
    if (source_id != nullptr)
        *source_id = id;
    /// update ctx
    ctx->sound_sources_[id] = source;

    /// Create audio processor for sound source
    int processor_id;
    if (is_async) {
        processor_id = ctx->audio_processors_->InsertAsync(mode, ctx);
    } else {
        processor_id = ctx->audio_processors_->Insert(mode, ctx);
    }
    if (processor_id != id)
        return ERROR;

    return SUCCESS;
}

result audio_set_source_gain(audio_context *ctx, int source_id, float gain) {
    if (gain < 0.0f) {
        return ILLEGAL_VALUE;
    }
    if (ctx->sound_sources_.find(source_id) == ctx->sound_sources_.end()) {
        return SOURCE_NOT_FOUND;
    }

    auto &processor = ctx->audio_processors_->at(source_id);
    if (processor.life_cycle != processor.kLifeCycle_Initialized)
        return ERROR;
    processor.object_ptr->source_gain = gain;
    return SUCCESS;
}


result audio_update_source_mode(audio_context *ctx,
                                int source_id,
                                source_mode mode) {
    if (ctx->sound_sources_.find(source_id) == ctx->sound_sources_.end())
        return SOURCE_NOT_FOUND;

    auto &processor = ctx->audio_processors_->at(source_id);
    if (processor.life_cycle != processor.kLifeCycle_Initialized)
        return ERROR;

    processor.object_ptr->source_mode_ = mode;
    return SUCCESS;
}

result audio_submit_source_buffer(audio_context *ctx,
                                  int source_id,
                                  const float *input_buffer_ptr,
                                  size_t num_frames) {
    if (!ctx->audio_processors_->Contains(source_id))
        return SOURCE_NOT_FOUND;

    auto &processor_entry = ctx->audio_processors_->at(source_id);
    if (processor_entry.life_cycle == processor_entry.kLifeCycle_Initializing)
        return SUCCESS;
    else if (processor_entry.life_cycle == processor_entry.kLifeCycle_Deleting) {
        ctx->audio_processors_->StopUsingObject(source_id);
        return SUCCESS;
    } else if (processor_entry.life_cycle == processor_entry.kLifeCycle_NotInitialized)
        return ERROR;
    if (!processor_entry.is_inuse)
        ctx->audio_processors_->StartUsingObject(source_id);

    auto processor = processor_entry.object_ptr;
    auto &source = ctx->sound_sources_.at(source_id);

    switch (processor->source_mode_) {
        case SOURCE_SPATIALIZE: {
            avs3renderer::Vector3f source_listener_vec = source->Position() - ctx->listener->Position();

            processor->direct_sound_encoder_->Process(
                    input_buffer_ptr, ctx->temp_ambisonic_buffer.data(), ctx->num_ambisonic_channels, num_frames,
                    (ctx->listener == nullptr ? Eigen::Matrix4f::Identity() : ctx->listener->WorldToListener())
                            .block<3, 3>(0, 0) * source_listener_vec,
                    processor->source_gain);
            break;
        }
        case SOURCE_BYPASS: {
            for (size_t frame = 0; frame < num_frames; ++frame) {
                // 0.71 for energy consistency
                ctx->temp_binaural_buffer[frame * 2 + 0] += input_buffer_ptr[frame] * 0.71 * processor->source_gain;
                ctx->temp_binaural_buffer[frame * 2 + 1] += input_buffer_ptr[frame] * 0.71 * processor->source_gain;
            }
            break;
        }
    }
    return SUCCESS;
}

result audio_get_interleaved_binaural_buffer(audio_context *ctx,
                                                       float *output_buffer_ptr,
                                                       size_t num_frames,
                                                       bool is_accumulative) {
    if (ctx->has_ambisonic_input) {
        ctx->ambisonic_rotator_->Process(ctx->temp_ambisonic_buffer_before_rotation.data(), ctx->num_ambisonic_channels,
                                         ctx->frames_per_buffer, ctx->temp_ambisonic_buffer_before_rotation.data());
        vraudio_simd::AddPointwise(ctx->temp_ambisonic_buffer.size(), ctx->temp_ambisonic_buffer.data(),
                                   ctx->temp_ambisonic_buffer_before_rotation.data(),
                                   ctx->temp_ambisonic_buffer.data());
        std::fill(ctx->temp_ambisonic_buffer_before_rotation.begin(), ctx->temp_ambisonic_buffer_before_rotation.end(),
                  0);
    }
    ctx->ambisonic_binaural_decoder_->Process(ctx->temp_ambisonic_buffer.data(), num_frames, output_buffer_ptr,
                                              is_accumulative);
    // Clean temp buffer
    std::fill(ctx->temp_ambisonic_buffer.begin(), ctx->temp_ambisonic_buffer.end(), 0);

    return SUCCESS;
}

result audio_get_planar_binaural_buffer(audio_context *ctx,
                                        float *const *output_buffer_ptr,
                                        size_t num_frames,
                                        bool is_accumulative) {
    if (ctx->has_ambisonic_input) {
        ctx->ambisonic_rotator_->Process(ctx->temp_ambisonic_buffer_before_rotation.data(), ctx->num_ambisonic_channels,
                                         ctx->frames_per_buffer, ctx->temp_ambisonic_buffer_before_rotation.data());
        vraudio_simd::AddPointwise(ctx->temp_ambisonic_buffer.size(), ctx->temp_ambisonic_buffer.data(),
                                   ctx->temp_ambisonic_buffer_before_rotation.data(),
                                   ctx->temp_ambisonic_buffer.data());
        std::fill(ctx->temp_ambisonic_buffer_before_rotation.begin(), ctx->temp_ambisonic_buffer_before_rotation.end(),
                  0);
    }
    ctx->ambisonic_binaural_decoder_->Process(ctx->temp_ambisonic_buffer.data(), num_frames, output_buffer_ptr,
                                              is_accumulative);

    // TODO(shijunjie): 之前只考虑了interleave的输出，所以 temp_binaural_buffer 是以 interleaved
    // 的格式存储的。如果支持planar的输出，中间的buffer 用 planar 可能会更方便一些，也少一些来回的计算。
    for (int f = 0; f < num_frames; ++f) {
        output_buffer_ptr[0][f] += ctx->temp_binaural_buffer[2 * f];
        output_buffer_ptr[1][f] += ctx->temp_binaural_buffer[2 * f + 1];
    }

    // Clean temp buffer
    std::fill(ctx->temp_ambisonic_buffer.begin(), ctx->temp_ambisonic_buffer.end(), 0);
    std::fill(ctx->temp_binaural_buffer.begin(), ctx->temp_binaural_buffer.end(), 0);
    return SUCCESS;
}

result audio_set_listener_position(audio_context *ctx, const float *position) {
    ctx->listener_params_.listener_position = {position[0], position[1], position[2]};
    ctx->listener->SetPosition(position[0], position[1], position[2]);
    return SUCCESS;
}

result audio_set_listener_orientation(audio_context *ctx,
                                      const float *front,
                                      const float *up) {
    ctx->listener_params_.listener_front = {front[0], front[1], front[2]};
    ctx->listener_params_.listener_up = {up[0], up[1], up[2]};
    ctx->listener->SetPose(ctx->listener_params_.listener_position, ctx->listener_params_.listener_front,
                           ctx->listener_params_.listener_up);
    if (ctx->has_ambisonic_input) {
        ctx->ambisonic_rotator_->SetRotationMatrix(
                (ctx->listener->WorldToListener() * ctx->ambisonic_orientation).block<3, 3>(0, 0));
    }
    return SUCCESS;
}

result audio_set_listener_pose(audio_context *ctx,
                               const float *position,
                               const float *front,
                               const float *up) {
    ctx->listener_params_.listener_position = {position[0], position[1], position[2]};
    ctx->listener_params_.listener_front = {front[0], front[1], front[2]};
    ctx->listener_params_.listener_up = {up[0], up[1], up[2]};
    ctx->listener->SetPose(ctx->listener_params_.listener_position, ctx->listener_params_.listener_front,
                           ctx->listener_params_.listener_up);
    if (ctx->has_ambisonic_input) {
        ctx->ambisonic_rotator_->SetRotationMatrix(
                (ctx->listener->WorldToListener() * ctx->ambisonic_orientation).block<3, 3>(0, 0));
    }
    return SUCCESS;
}

result audio_set_source_position(audio_context *ctx,
                                 int source_id,
                                 const float *position) {
    if (ctx->sound_sources_.find(source_id) == ctx->sound_sources_.end()) {
        return SOURCE_NOT_FOUND;
    } else {
        ctx->sound_sources_[source_id]->SetPosition(position[0], position[1], position[2]);
    }
    return SUCCESS;
}

result audio_commit_scene(audio_context *ctx) {
    if (ctx == nullptr) {
        return CONTEXT_NOT_CREATED;
    }
    return SUCCESS;
}

result audio_destroy(audio_context *ctx) {
    if (ctx != nullptr) {
        //  Join rendering threads
        delete ctx;
        return SUCCESS;
    }
    return ILLEGAL_VALUE;
}

result audio_submit_ambisonic_channel_buffer(audio_context *ctx,
                                             const float *ambisonic_channel_buffer,
                                             int order,
                                             int degree,
                                             ambisonic_normalization_type norm_type,
                                             float gain) {
    ctx->has_ambisonic_input = true;
    if (order > ctx->ambisonic_order)
        return ILLEGAL_VALUE;
    int acn_channel = avs3renderer::AcnSequence(order, degree);
    if (norm_type == N3D)
        gain *= avs3renderer::KN3d2Sn3d(acn_channel);
    vraudio_simd::ScalarMultiplyAndAccumulate(ctx->frames_per_buffer, gain, ambisonic_channel_buffer,
                                              ctx->temp_ambisonic_buffer_before_rotation.data() + acn_channel * ctx->frames_per_buffer);

    return SUCCESS;
}

result audio_submit_interleaved_ambisonic_buffer(audio_context *ctx,
                                                 const float *ambisonic_buffer,
                                                 int ambisonic_order,
                                                 ambisonic_normalization_type norm_type,
                                                 float gain) {
    ctx->has_ambisonic_input = true;
    if (ambisonic_order > ctx->ambisonic_order)
        return ILLEGAL_VALUE;

    for (int c = 0; c < avs3renderer::kNumAmbisonicChannels[ambisonic_order]; ++c) {
        float channel_gain = (norm_type == N3D) ? gain : gain *= avs3renderer::KN3d2Sn3d(c);
        for (int f = 0; f < ctx->frames_per_buffer; ++f) {
            ctx->temp_ambisonic_buffer_before_rotation[c * ctx->frames_per_buffer + f] +=
                    ambisonic_buffer[f * avs3renderer::kNumAmbisonicChannels[ambisonic_order] + c] * channel_gain;
        }
    }
    return SUCCESS;
}