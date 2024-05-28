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

#ifndef SPHERICAL_HARMONICS_H
#define SPHERICAL_HARMONICS_H

#include <cmath>
#include "SHEval.h"
#include "spherical_harmonics_constants.h"

namespace avs3renderer {

/**
 * Returns ACN channel sequence from an order and degree of a spherical harmonic.
 *
 * ACN = n^2+n+m
 * @param order
 * @param degree
 * @return ACN channel sequence
 */

inline int AcnSequence(int n /* n */, int m /* m */) {
    return n * n + n + m;
}

inline int OrderFromAcnSequence(int channel) {
    return static_cast<int>(std::floor(std::sqrt(static_cast<float>(channel))));
}

inline int DegreeFromAcnSequence(int channel) {
    int order = OrderFromAcnSequence(channel);
    return channel - order * order - order;
}

inline float KN3d2Sn3d(int n) {
    int order = OrderFromAcnSequence(n);
    return 1.0f / sqrtf(2.0f * order + 1);
}

/**
 * Real Spherical harmonics of AudioPosition (+x front, +y left, +z up)
 * @param order ambisonic order
 * @param x X Position in Audio Coordinate
 * @param y Y Position in Audio Coordinate
 * @param z Y Position in Audio Coordinate
 * @param[out] Ynm
 * @param sn3d true for SN3D normalization, false for N3D
 * @param encoding
 */
inline void RealSphericalHarmonics(const int order,
                                   const float x,
                                   const float y,
                                   const float z,
                                   float* Ynm) {
    // Sloan2013SH is N3D normalization, without Condon–Shortley phase.
    switch (order) {
        case 0:
            Sloan2013SH::SHEval0(x, y, z, Ynm);
            break;
        case 1:
            Sloan2013SH::SHEval1(x, y, z, Ynm);
            break;
        case 2:
            Sloan2013SH::SHEval2(x, y, z, Ynm);
            break;
        case 3:
            Sloan2013SH::SHEval3(x, y, z, Ynm);
            break;
        case 4:
            Sloan2013SH::SHEval4(x, y, z, Ynm);
            break;
        case 5:
            Sloan2013SH::SHEval5(x, y, z, Ynm);
            break;
        case 6:
            Sloan2013SH::SHEval6(x, y, z, Ynm);
            break;
        case 7:
            Sloan2013SH::SHEval7(x, y, z, Ynm);
            break;
    }
    float encoding_scale, norm_scale;
    for (int n = 0; n < kNumAmbisonicChannels[order]; ++n) {
        encoding_scale = SQRT4PI;
        norm_scale = KN3d2Sn3d(n);
        Ynm[n] = Ynm[n] * encoding_scale * norm_scale;
    }
}

}  // namespace avs3renderer

#endif  // SPHERICAL_HARMONICS_H
