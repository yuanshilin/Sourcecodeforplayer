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
#include "ext/simd/simd_utils.h"

#include <cmath>

namespace avs3renderer {

inline Vector3f AudioPositionFromWorldPosition(const Vector3f& p) {
    return {p.z(), p.x(), p.y()};
}

inline Transform4f ToWorldMatrix(const Point3f& position, const Vector3f& front, const Vector3f& up) {
    Transform4f to_world;
    to_world(0, 3) = position.x();
    to_world(1, 3) = position.y();
    to_world(2, 3) = position.z();
    to_world(3, 3) = 1.0f;

    if (front.cross(up).norm() == 0.f) {
        return to_world;
    }
    Vector3f new_z = front.normalized();
    Vector3f new_x = up.cross(new_z).normalized();
    Vector3f new_y = new_z.cross(new_x);

    to_world(0, 0) = new_x.x();
    to_world(1, 0) = new_x.y();
    to_world(2, 0) = new_x.z();
    to_world(3, 0) = 0.f;

    to_world(0, 1) = new_y.x();
    to_world(1, 1) = new_y.y();
    to_world(2, 1) = new_y.z();
    to_world(3, 1) = 0.f;

    to_world(0, 2) = new_z.x();
    to_world(1, 2) = new_z.y();
    to_world(2, 2) = new_z.z();
    to_world(3, 2) = 0.f;

    return to_world;
}

// @param input Integer value.
// @return The next power of two from |input|.
inline size_t NextPowTwo(size_t input) {
    // Ensure the value fits in a uint32_t.
    uint32_t number = static_cast<uint32_t>(--input);
    number |= number >> 1;   // Take care of 2 bit numbers.
    number |= number >> 2;   // Take care of 4 bit numbers.
    number |= number >> 4;   // Take care of 8 bit numbers.
    number |= number >> 8;   // Take care of 16 bit numbers.
    number |= number >> 16;  // Take care of 32 bit numbers.
    number++;
    return static_cast<size_t>(number);
}

inline float FastReciprocalSqrt(float input) {
    const float kThreeHalfs = 1.5f;
    const uint32_t kMagicNumber = 0x5f3759df;

    // Approximate a logarithm by aliasing to an integer.
    uint32_t integer = *reinterpret_cast<uint32_t*>(&input);
    integer = kMagicNumber - (integer >> 1);
    float approximation = *reinterpret_cast<float*>(&integer);
    const float half_input = input * 0.5f;
    // One iteration of Newton's method.
    return approximation * (kThreeHalfs - (half_input * approximation * approximation));
}

template <typename MatrixType>
Eigen::Matrix<typename MatrixType::Scalar, MatrixType::ColsAtCompileTime, MatrixType::RowsAtCompileTime> Pseudoinverse(
    const MatrixType& matrix) {
    Eigen::JacobiSVD<Eigen::Matrix<typename MatrixType::Scalar, Eigen::Dynamic, Eigen::Dynamic>> svd(
        matrix, Eigen::ComputeThinU | Eigen::ComputeThinV);
    return svd.solve(Eigen::Matrix<typename MatrixType::Scalar, MatrixType::RowsAtCompileTime,
                                   MatrixType::RowsAtCompileTime>::Identity(matrix.rows(), matrix.rows()));
}

inline void ConvertSampleToFloatFormat(int16_t input, float* output) {
    static const float kInt16Max = static_cast<float>(0x7FFF);
    static const float kInt16ToFloat = 1.0f / kInt16Max;
    *output = input * kInt16ToFloat;
}

inline size_t CeilToMultipleOfFramesPerBuffer(size_t size, size_t frames_per_buffer) {
    const size_t remainder = size % frames_per_buffer;
    return remainder == 0 ? std::max(size, frames_per_buffer) : size + frames_per_buffer - remainder;
}

}  // namespace avs3renderer