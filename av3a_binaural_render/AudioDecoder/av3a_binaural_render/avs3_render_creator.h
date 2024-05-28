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

#ifndef AVS3_RENDER_AVS3_RENDER_CREATOR_H
#define AVS3_RENDER_AVS3_RENDER_CREATOR_H

#include "avs3_render.h"
#include "binaural_render.h"

/***
 * @param id renderer id, one of enum AVS3RenderID
 * @param metadata Metadata contains ADM's element, it will better match renderer
 * @param mode File mode for bw64 file, Stream for stream mode
 * @return renderer point
 */

template<typename T>
static AVS3::AVS3Render<T> *createRenderByID(AVS3::AVS3RenderID id,
                                             std::shared_ptr<T> &metadata,
                                             AVS3::RenderMode mode = AVS3::File) {

    switch (id) {
        case AVS3::Binaural_Render:
            return new AVS3::BinauralRender(metadata, mode);
        case AVS3::Other:
        default:
            return nullptr;
    }
}

#endif //AVS3_RENDER_AVS3_RENDER_CREATOR_H
