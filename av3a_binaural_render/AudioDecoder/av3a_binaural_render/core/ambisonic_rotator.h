/*
Copyright 2018 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef AMBISONIC_ROTATOR_H
#define AMBISONIC_ROTATOR_H
#include <cstdlib>
#include <vector>
#include <atomic>
#include "Eigen/Dense"

namespace avs3renderer {
class WorldRotation : public Eigen::Quaternion<float, Eigen::DontAlign> {
public:
    // Inherits all constructors with 1-or-more arguments. Necessary because
    // MSVC12 doesn't support inheriting constructors.
    template <typename Arg1, typename... Args>
    WorldRotation(const Arg1& arg1, Args&&... args) : Quaternion(arg1, std::forward<Args>(args)...) {
    }

    // Constructs an identity rotation.
    WorldRotation() {
        setIdentity();
    }

    // Returns the shortest arc between two |WorldRotation|s in radians.
    float AngularDifferenceRad(const WorldRotation& other) const {
        const Quaternion difference = this->inverse() * other;
        return static_cast<float>(Eigen::AngleAxisf(difference).angle());
    }
};
class AmbisonicRotator {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    AmbisonicRotator(int ambisonic_order);

    /**
     * Set the Yaw, Pitch, Roll status (not motion) in degree, position represent anti-clockwise rotation.
     * @param yaw rotation against Y axis (Z in Audio coordinate)
     * @param pitch rotation against X axis (Y in Audio coordinate)
     * @param roll rotation against Z (X in Audio coordinate)
     */
    void SetYawPitchRoll(float yaw, float pitch, float roll);

    /**
     * Set the target rotation matrix.
     * @param rotation_matrix
     */
    void SetRotationMatrix(const Eigen::Matrix3f& rotation_matrix);

    /**
     * Rotate Ambisonic buffer. This method will OVERWRITE data at output_buffer_ptr.
     * @param input_buffer_ptr Planar input Ambisonics buffer
     * @param num_channels Number of channels, should equal to ambisonic channels
     * @param num_frames Number of frames
     * @param output_buffer_ptr Planar output Ambisonics buffer
     */
    void Process(const float* input_buffer_ptr, size_t num_channels, size_t num_frames, float* output_buffer_ptr);

private:
    void UpdateRotationMatrix(const Eigen::Quaternion<float>& rotation);

    int ambisonic_order_;

    // Current rotation which is used in the interpolation process in order to
    // compute new rotation matrix. Initialized with an identity rotation.
    WorldRotation current_rotation_;              ///< only write in process thread
    std::vector<WorldRotation> target_rotation_;  ///< write in setting thread, read in process thread
    std::atomic<int> target_rotation_read_index_;

    // Spherical harmonics rotation sub-matrices for each order.
    std::vector<Eigen::MatrixXf> rotation_matrices_;

    // Final spherical harmonics rotation matrix.
    Eigen::MatrixXf rotation_matrix_;
};
}  // namespace avs3renderer

#endif  // AMBISONIC_ROTATOR_H
