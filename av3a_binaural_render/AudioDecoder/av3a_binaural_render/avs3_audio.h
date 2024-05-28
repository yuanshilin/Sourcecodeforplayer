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

#ifndef API_AUDIO_H
#define API_AUDIO_H

#include <cstdint>
#include <cstdlib>
#include "avs3_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define PATCH_VERSION 2

/**
 * Returns version of Renderer
 *
 * @param[out] major
 * @param[out] minor
 * @param[out] patch
 * @return
 */
const char* get_version(int* major, int* minor, int* patch);

/**
 * * Create Renderer context
 *
 * All API calls of Renderer engine needs to be provided with a context，therefore you should create a context BEFORE you make any API calls
 *
 * @param[out] ctx A pointer of context pointer. The created context address will be written to the context pointer
 * @param mode The rendering tier for your specific need, you can choose from one of the following
 * tiers:
 *       AMBISONIC_FIRST_ORDER,
 *       ...,
 *       AMBISONIC_SEVENTH_ORDER.
 * If you build / run in free field mode, this rendering tier only affects the ambisonic order;
 * however, if you included environmental acoustics simulation, this rendering tier also changes number of rays,
 * maximum path depth, and etc.
 * @param frames_per_buffer which defines the size of each mono input buffer. More details about the concept of "audio
 * frame" can be found here:
 * https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API/Basic_concepts_behind_Web_Audio_API#audio_buffers_frames_samples_and_channels
 * @param sample_rate The sampling rate of I/O signal to/from the engine.
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_create_context(audio_context** ctx,
                            rendering_mode mode,
                            size_t frames_per_buffer,
                            size_t sample_rate);

/**
 * Initialize Renderer context
 *
 * Must be called
 * 1. After creating a context using audio_create_context
 * 2. Before make other calls
 * Before calling this API, you can further modify configs of the context manually after it's created;
 *
 * @param ctx Renderer context to be initialized.
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_initialize_context(audio_context* ctx);

/**
 * Submits acoustic mesh to the scene
 *
 * When using the Renderer engine with environmental acoustics simulation, you need to provide the engine with scene mesh and material information.
 * If you are using free field mode (without environmental acoustics simulation), you don't need to call this API; It won't do anything.
 *
 * @param ctx Renderer context
 * @param vertices Vertices array of the scene, in the layout of [x0 y0 z0 x1 y1 z1 ...]
 * @param vertices_count Number of vertices in the vertices array
 * @param indices Indices array of the scene, which contains groups of 3 vertex indices that forms a triangle
 * @param indices_count Number of triangles represented by the indices array
 * @param material The acoustic material enum of this scene mesh. See definition of acoustics_material for all categories
 * @param[out] geometry_id Returns the geometry id of the submitted scene mesh, this is unique in current current
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_submit_mesh(audio_context* ctx,
                         const float* vertices,
                         int vertices_count,
                         const int* indices,
                         int indices_count,
                         int material,
                         int* geometry_id);

/**
 * Submits acoustic mesh to the scene. This API has the same functionality as audio_submit_mesh, but can directly input absorption and scattering factor of material
 * @param ctx Renderer context
 * @param vertices Vertices array of the scene
 * @param vertices_count Number of vertices in the vertices array
 * @param indices Indices array of the scene
 * @param indices_count Number of triangles represented by the indices array
 * @param absorption_factor Absorption factor of the material in all frequency bands
 * @param scattering_factor Scattering factor of the material
 * @param[out] geometry_id Returns the geometry id of the submitted scene mesh, this is unique in current current
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_submit_mesh_and_material_factor(audio_context* ctx,
                                             const float* vertices,
                                             int vertices_count,
                                             const int* indices,
                                             int indices_count,
                                             const float* absorption_factor,
                                             float scattering_factor,
                                             int* geometry_id);

/**
 * Commit current scene to Renderer engine
 *
 * After calling audio_submit_mesh or audio_initialize_context, you need to call this API to
 * include the scene into computation.
 * You should ALSO call this method even there's no acoustic mesh in the scene。
 *
 * @param Renderer context
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_commit_scene(audio_context* ctx);

/**
 * Create sound source
 *
 * Create a virtual sound source, and set it up with specified source mode and position in scene. Returns source id.
 *
 * @param Renderer context
 * @param source_mode Source rendering mode, which currently includes SOURCE_SPATIALIZE(default) and
 * SOURCE_BYPASS. SOURCE_SPATIALIZE will let this sound source be spatialized, while SOURCE_BYPASS will
 * directly mix input signal to output channels
 * @param position Source world position, which is a 3-element float array [x, y, z] follows the convention defined in
 * README.md. Unit is meter
 * @param[out] source_id Returns unique source id.
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_add_source(audio_context* ctx,
                        source_mode source_mode,
                        const float* position,
                        int* source_id,
                        bool is_async = false);

/**
 * Same as audio_add_source, except this API can setup source orientation and ray tracing detection radius.
 * Source orientation is useful when creating non-omnidirectional sources (non-omnidirectional source simulation is
 * still under development).
 * Ray tracing detection radius affects echo density and authenticity of simulation early reflections and late reverb
 *
 * @param Renderer context
 * @param source_mode Source rendering mode, which currently includes SOURCE_SPATIALIZE(default) and
 * SOURCE_BYPASS. SOURCE_SPATIALIZE will let this sound source be spatialized, while SOURCE_BYPASS will
 * directly mix input signal to output channels
 * @param position Source world position, which is a 3-element float array [x, y, z] follows the convention defined in
 * README.md. Unit is meter
 * @param front Facing diretcion of the source, which defaults to positive direction of Z axis, [0, 0, 1]
 * @param up Direction vector that defines where the top of the source is facing at. it defaults to positive direction
 * of Y axis, [0, 1, 0]
 * @param[out] source_id Returns unique source id.
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_add_source_with_orientation(audio_context* ctx,
                                         source_mode mode,
                                         const float* position,
                                         const float* front,
                                         const float* up,
                                         int* source_id,
                                         bool is_async = false);

/**
 * @param ctx 要使用的的 context
 * @param source_id Id of sound source
 * @param input_buffer_ptr Pointer to mono audio buffer
 * @param num_frames Number of frames in audio buffer，通常这个参数的值应当与创建 context 时的 frames_per_buffer 一致。
 * @return 返回执行结果
 *
 * submit input audio for source with specified source ID
 *
 * This API is one of the core API calls of Renderer engine.
 * In realtime audio loops, we need to submit source signals of ALL sound sources in the scene for the engine to compute.
 * Mind that this API do take some time to process, but it will meet the realtime deadline with ample head room in time.
 *
 * @param Renderer context
 * @param source_id Id of sound source
 * @param input_buffer_ptr Pointer to mono audio buffer
 * @param num_frames Number of frames in audio buffer, usually the same value as the frames_per_buffer used when
 * creating Renderer context.
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_submit_source_buffer(audio_context* ctx,
                                  int source_id,
                                  const float* input_buffer_ptr,
                                  size_t num_frames);
/**
 * Submit a channel of HOA input signal, with input order and degree indices to specify ambisonic channel. The HOA input
 * here will be mixed with object-based audio output
 *
 * @param Renderer context
 * @param ambisonic_channel_buffer The buffer of one HOA channel
 * @param order The order index of this HOA channel
 * @param degree The degree index of this HOA channel
 * @param norm_type The normalization method used for this HOA channel, for example SN3D or N3D
 * @param gain The gain/attenuation added to this HOA channel. Normally all channels of an HOA should have the same
 * gain.
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_submit_ambisonic_channel_buffer(audio_context* ctx,
                                             const float* ambisonic_channel_buffer,
                                             int order,
                                             int degree,
                                             ambisonic_normalization_type norm_type,
                                             float gain);

/**
 * Submit a interleaved buffer of HOA input signal. The HOA input here will be mixed with object-based audio output
 *
 * @param Renderer context
 * @param ambisonic_buffer Interleaved HOA input buffer
 * @param ambisonic_order Order of the input HOA signal
 * @param norm_type The normalization method used for this HOA channel, for example SN3D or N3D
 * @param gain The gain/attenuation added to this HOA channel. Normally all channels of an HOA should have the same
 * gain.
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_submit_interleaved_ambisonic_buffer(audio_context* ctx,
                                                 const float* ambisonic_buffer,
                                                 int ambisonic_order,
                                                 ambisonic_normalization_type norm_type,
                                                 float gain);

/**
 * Get the interleaved output of the spatialized binaural output
 *
 * This is one of the core API call of Renderer engine. After we submitted all source input using
 * audio_submit_source_buffer, use this API to get the interleaved binaural output signal.
 * FYI: use headphone to play back binaural signal.
 *
 * @param Renderer context
 * @param output_buffer_ptr Pointer to output audio buffer, which is a interleaved stereo buffer with
 * 2 * frames_per_buffer of element.
 * @param num_frames Frame count of the output signal。
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_get_interleaved_binaural_buffer(audio_context* ctx,
                                             float* output_buffer_ptr,
                                             size_t num_frames,
                                             bool is_accumulative = false);

/**
 * Same as audio_get_interleaved_binaural_buffer, but the output layout is planer stereo.
 *
 * This is one of the core API call of engine. After we submitted all source input using
 * audio_submit_source_buffer, use this API to get the planer binaural output signal.
 * FYI: use headphone to play back binaural signal.
 *
 * @param Renderer context
 * @param output_buffer_ptr Pointer to output audio buffer, which is a planer stereo buffer with
 * 2 * frames_per_buffer of element.
 * @param num_frames Frame count of the output signal
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_get_planar_binaural_buffer(audio_context* ctx,
                                        float* const* output_buffer_ptr,
                                        size_t num_frames,
                                        bool is_accumulative = false);

/**
 * Update acoustic scene status
 *
 * One of the core API call of Renderer engine. It has to be called frequently in order to refresh audio rendering
 * parameters inside engine.
 * In free field mode, calling this API would update source and listener status.
 * When environmental acoustics is enabled, this API would also update the influence of scene on early reflection
 * and late reverb.
 * Also, in free field mode, this API can be called in audio thread; However, when environmental acoustics is enabled,
 * you should never call it in audio thread, or any other thread with hard deadline.
 *
 * @param ctx Renderer context
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_update_scene(audio_context* ctx);

/**
 * Setup world position of listener. This API call wouldn't change the orientation of the listener
 *
 * @param ctx Renderer context
 * @param position Pointer to a 3-float array containing the world position of listener in XYZ order (unit: meter).
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_set_listener_position(audio_context* ctx, const float* position);

/**
 * Setup world orientation of the listener. This API call wouldn't change the position of the listener
 *
 * @param ctx Renderer context
 * @param front Facing direction of the source.
 * @param up Direction vector that defines where the top of the source is facing at.
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_set_listener_orientation(audio_context* ctx,
                                      const float* front,
                                      const float* up);

/**
 * Setup both world position and orientation of the listener
 *
 * @param ctx Renderer context
 * @param position Pointer to a 3-float array containing the world position of listener in XYZ order (unit: meter).
 * @param front Facing direction of the source.
 * @param up Direction vector that defines where the top of the source is facing at.
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_set_listener_pose(audio_context* ctx,
                               const float* position,
                               const float* front,
                               const float* up);

/**
 * Setup the world position of the specified source
 *
 * @param ctx Renderer context
 * @param source_id The ID of the source to modify
 * @param position Pointer to a 3-float array containing the world position of listener in XYZ order (unit: meter).
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_set_source_position(audio_context* ctx,
                                 int source_id,
                                 const float* position);

/**
 * Setup gain applied to the specified source
 *
 * @param ctx Renderer context
 * @param source_id The ID of the source to modify
 * @param gain Source signal gain/attenuation
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_set_source_gain(audio_context* ctx, int source_id, float gain);

/**
 * Update source mode: between SOURCE_SPATIALIZE or SOURCE_BYPASS
 * @param ctx Renderer context
 * @param source_id The ID of the source to modify
 * @param mode Source playback mode
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_update_source_mode(audio_context* ctx,
                                int source_id,
                                source_mode mode);

/**
 * Destroy context
 *
 * After done running the Renderer engine, call this method to deconstruct context
 *
 * @param ctx Renderer context
 * @return Error code of this API call. Returns SUCCESS if nothing was wrong.
 */
result audio_destroy(audio_context* ctx);

#ifdef __cplusplus
}
#endif

#endif  // API_AUDIO_H
