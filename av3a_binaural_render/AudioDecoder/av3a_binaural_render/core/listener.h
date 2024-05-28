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
#include <unordered_map>
#include <vector>
#include <thread>
#include <atomic>
#include "core/definitions.h"
#include "sound_source.h"

namespace avs3renderer {
class Spectrum;

class Listener {
protected:
    /**
     * | left.x up.x front.x pos.x |
     * | left.y up.y front.y pos.y |
     * | left.z up.z front.z pos.z |
     * | 0      0    0       0     |
     */
    std::vector<Transform4f> to_world_;
    std::atomic<int> to_world_read_index_;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    explicit Listener(const Transform4f& to_world = Transform4f::Identity())
        : to_world_(2, to_world), to_world_read_index_(0) {
    }

    Listener(const Point3f& position, const Vector3f& front, const Vector3f& up);

    virtual ~Listener() = default;

    virtual void SetPosition(float x, float y, float z);

    // Set listener's pose and return the listener to world transform matrix.
    virtual void SetPose(const Point3f& position, const Vector3f& front, const Vector3f& up);

    virtual Point3f Position() const {
        return to_world_.at(to_world_read_index_.load()).block<3, 1>(0, 3);
    }

    virtual float* PositionPtr() {
        return to_world_.at(to_world_read_index_.load()).block<3, 1>(0, 3).data();
    }

    virtual float* FrontPtr() {
        return to_world_.at(to_world_read_index_.load()).block<3, 1>(0, 2).data();
    }

    virtual Vector3f ToWorldDirection(const Vector3f& direction) const {
        Eigen::Vector4f dir = {direction.x(), direction.y(), direction.z(), 0.f};
        Eigen::Vector4f w_dir = to_world_.at(to_world_read_index_.load()) * dir;
        return {w_dir.x(), w_dir.y(), w_dir.z()};
    }

    Transform4f ListenerToWorld() const {
        return to_world_.at(to_world_read_index_.load());
    }

    Transform4f WorldToListener() const {
        return to_world_.at(to_world_read_index_.load()).inverse();
    }
};

class OmniListener : public Listener {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    explicit OmniListener(const Transform4f& to_world = Transform4f::Identity());
    OmniListener(const Point3f& position, const Vector3f& front, const Vector3f& up);
};
}  // namespace avs3renderer
