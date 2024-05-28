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

#pragma once
#include "core/definitions.h"
#include <atomic>

namespace avs3renderer {

class SoundSource {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    SoundSource()
        : id_(IdGenerator()), to_world_(2, Transform4f::Identity()), to_world_read_index_(0) {
    }
    SoundSource(const Transform4f& to_world, int source_id = -1)
        : id_(source_id >= 0? source_id : IdGenerator()), to_world_(2, to_world), to_world_read_index_(0) {
    }
    SoundSource(const Point3f& position, const Vector3f& front, const Vector3f& up, int source_id = -1);
    virtual void SetPosition(float x, float y, float z);
    virtual Point3f Position() const;
    virtual void SetPose(const Point3f& position, const Vector3f& front, const Vector3f& up);
    int id() const {
        return id_;
    };

protected:
    std::vector<Transform4f> to_world_;
    std::atomic<int> to_world_read_index_;

private:
    static int IdGenerator();
    int id_;
};

class OmniSoundSource : public SoundSource {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    explicit OmniSoundSource(const Transform4f& to_world = Transform4f::Identity(),
                             int source_id = -1);

    OmniSoundSource(const Point3f& position,
                    const Vector3f& front,
                    const Vector3f& up,
                    int source_id = -1);
    void SetPosition(float x, float y, float z) override;
    void SetPose(const Point3f& position, const Vector3f& front, const Vector3f& up) override;
};

}  // namespace avs3renderer
