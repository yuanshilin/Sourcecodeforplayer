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

#ifndef AVS3_RENDER_AVS3_RENDER_H
#define AVS3_RENDER_AVS3_RENDER_H

#include <memory>
#include <metadata.h>

namespace AVS3 {

typedef enum {
    Binaural_Render,
    Other
} AVS3RenderID;

typedef enum {
    File,
    Stream
} RenderMode;


template<typename T>
class AVS3Render {

public:

    /***
     * @param input input interleaved buffer
     * @param channelCount input channel count
     * @param sampleNum input sample count
     * @return 0 success else fail
     */
    virtual int putAudioData(float **input, int channelCount, int sampleNum) = 0;

    /***
     * @param input input planar buffer
     * @param channelCount input channel count
     * @param sampleNum input sample count
     * @return  0 success else fail
     */
    virtual int putAudioData(float *input, int channelCount, int sampleNum) = 0;

    /***
     * @param output out interleaved buffer
     * @param sampleCount out sample count
     * @return  0 success else fail
     */
    virtual int getAudioData(float **output, int sampleCount) = 0;

    /***
     * @param output out planar buffer
     * @param sampleCount out sample count
     * @return  0 success else fail
     */
    virtual int getAudioData(float *output, int sampleCount) = 0;

    /***
     * @brief for stream mode, dynamic metadata interface
     * @param position
     * @param trackIndex
     * @param trackNum
     * @return  0 success else fail
     */
    virtual int setObjectPosition(float **position, int* trackIndex, int trackNum) = 0;

    /***
     * @param position
     * @param front
     * @param up
     * @return  0 success else fail
     */
    virtual int setListenerPosition(float *position, float *front, float *up) {
        return 0;
    };

    /***
     * @brief release renderer
     * @return  0 success else fail
     */
    virtual int destroyRender() = 0;

    virtual ~AVS3Render() = default;

};
}

#endif //AVS3_RENDER_AVS3_RENDER_H
