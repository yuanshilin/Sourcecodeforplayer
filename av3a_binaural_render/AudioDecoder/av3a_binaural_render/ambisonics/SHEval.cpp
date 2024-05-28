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
//================================================================================================
/*
 The following functions were generated with the Code-Generator by Peter-Pike Sloan.
 More information about the algorithm can be found here:  http://jcgt.org/published/0002/02/06/
 */

namespace avs3renderer {
namespace Sloan2013SH {

// order 0
void SHEval0(const float fX, const float fY, const float fZ, float* pSH) {
    pSH[0] = 0.2820947917738781f;
}

// order 1
void SHEval1(const float fX, const float fY, const float fZ, float* pSH) {
    float fC0, fS0, fTmpC;

    pSH[0] = 0.2820947917738781f;
    pSH[2] = 0.4886025119029199f * fZ;
    fC0 = fX;
    fS0 = fY;

    fTmpC = 0.48860251190292f;
    pSH[3] = fTmpC * fC0;
    pSH[1] = fTmpC * fS0;
}

// order 2
void SHEval2(const float fX, const float fY, const float fZ, float* pSH) {
    float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
    float fZ2 = fZ * fZ;

    pSH[0] = 0.2820947917738781f;
    pSH[2] = 0.4886025119029199f * fZ;
    pSH[6] = 0.9461746957575601f * fZ2 + -0.31539156525252f;
    fC0 = fX;
    fS0 = fY;

    fTmpA = 0.48860251190292f;
    pSH[3] = fTmpA * fC0;
    pSH[1] = fTmpA * fS0;
    fTmpB = 1.092548430592079f * fZ;
    pSH[7] = fTmpB * fC0;
    pSH[5] = fTmpB * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpC = 0.5462742152960395f;
    pSH[8] = fTmpC * fC1;
    pSH[4] = fTmpC * fS1;
}

// order 3
void SHEval3(const float fX, const float fY, const float fZ, float* pSH) {
    float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
    float fZ2 = fZ * fZ;

    pSH[0] = 0.2820947917738781f;
    pSH[2] = 0.4886025119029199f * fZ;
    pSH[6] = 0.9461746957575601f * fZ2 + -0.31539156525252f;
    pSH[12] = fZ * (1.865881662950577f * fZ2 + -1.119528997770346f);
    fC0 = fX;
    fS0 = fY;

    fTmpA = 0.48860251190292f;
    pSH[3] = fTmpA * fC0;
    pSH[1] = fTmpA * fS0;
    fTmpB = 1.092548430592079f * fZ;
    pSH[7] = fTmpB * fC0;
    pSH[5] = fTmpB * fS0;
    fTmpC = 2.285228997322329f * fZ2 + -0.4570457994644658f;
    pSH[13] = fTmpC * fC0;
    pSH[11] = fTmpC * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.5462742152960395f;
    pSH[8] = fTmpA * fC1;
    pSH[4] = fTmpA * fS1;
    fTmpB = 1.445305721320277f * fZ;
    pSH[14] = fTmpB * fC1;
    pSH[10] = fTmpB * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpC = 0.5900435899266435f;
    pSH[15] = fTmpC * fC0;
    pSH[9] = fTmpC * fS0;
}

// order 4
void SHEval4(const float fX, const float fY, const float fZ, float* pSH) {
    float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
    float fZ2 = fZ * fZ;

    pSH[0] = 0.2820947917738781f;
    pSH[2] = 0.4886025119029199f * fZ;
    pSH[6] = 0.9461746957575601f * fZ2 + -0.31539156525252f;
    pSH[12] = fZ * (1.865881662950577f * fZ2 + -1.119528997770346f);
    pSH[20] = 1.984313483298443f * fZ * pSH[12] + -1.006230589874905f * pSH[6];
    fC0 = fX;
    fS0 = fY;

    fTmpA = 0.48860251190292f;
    pSH[3] = fTmpA * fC0;
    pSH[1] = fTmpA * fS0;
    fTmpB = 1.092548430592079f * fZ;
    pSH[7] = fTmpB * fC0;
    pSH[5] = fTmpB * fS0;
    fTmpC = 2.285228997322329f * fZ2 + -0.4570457994644658f;
    pSH[13] = fTmpC * fC0;
    pSH[11] = fTmpC * fS0;
    fTmpA = fZ * (4.683325804901025f * fZ2 + -2.007139630671868f);
    pSH[21] = fTmpA * fC0;
    pSH[19] = fTmpA * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.5462742152960395f;
    pSH[8] = fTmpA * fC1;
    pSH[4] = fTmpA * fS1;
    fTmpB = 1.445305721320277f * fZ;
    pSH[14] = fTmpB * fC1;
    pSH[10] = fTmpB * fS1;
    fTmpC = 3.31161143515146f * fZ2 + -0.47308734787878f;
    pSH[22] = fTmpC * fC1;
    pSH[18] = fTmpC * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpA = 0.5900435899266435f;
    pSH[15] = fTmpA * fC0;
    pSH[9] = fTmpA * fS0;
    fTmpB = 1.770130769779931f * fZ;
    pSH[23] = fTmpB * fC0;
    pSH[17] = fTmpB * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpC = 0.6258357354491763f;
    pSH[24] = fTmpC * fC1;
    pSH[16] = fTmpC * fS1;
}

// order 5
void SHEval5(const float fX, const float fY, const float fZ, float* pSH) {
    float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
    float fZ2 = fZ * fZ;

    pSH[0] = 0.2820947917738781f;
    pSH[2] = 0.4886025119029199f * fZ;
    pSH[6] = 0.9461746957575601f * fZ2 + -0.31539156525252f;
    pSH[12] = fZ * (1.865881662950577f * fZ2 + -1.119528997770346f);
    pSH[20] = 1.984313483298443f * fZ * pSH[12] + -1.006230589874905f * pSH[6];
    pSH[30] = 1.98997487421324f * fZ * pSH[20] + -1.002853072844814f * pSH[12];
    fC0 = fX;
    fS0 = fY;

    fTmpA = 0.48860251190292f;
    pSH[3] = fTmpA * fC0;
    pSH[1] = fTmpA * fS0;
    fTmpB = 1.092548430592079f * fZ;
    pSH[7] = fTmpB * fC0;
    pSH[5] = fTmpB * fS0;
    fTmpC = 2.285228997322329f * fZ2 + -0.4570457994644658f;
    pSH[13] = fTmpC * fC0;
    pSH[11] = fTmpC * fS0;
    fTmpA = fZ * (4.683325804901025f * fZ2 + -2.007139630671868f);
    pSH[21] = fTmpA * fC0;
    pSH[19] = fTmpA * fS0;
    fTmpB = 2.03100960115899f * fZ * fTmpA + -0.991031208965115f * fTmpC;
    pSH[31] = fTmpB * fC0;
    pSH[29] = fTmpB * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.5462742152960395f;
    pSH[8] = fTmpA * fC1;
    pSH[4] = fTmpA * fS1;
    fTmpB = 1.445305721320277f * fZ;
    pSH[14] = fTmpB * fC1;
    pSH[10] = fTmpB * fS1;
    fTmpC = 3.31161143515146f * fZ2 + -0.47308734787878f;
    pSH[22] = fTmpC * fC1;
    pSH[18] = fTmpC * fS1;
    fTmpA = fZ * (7.190305177459987f * fZ2 + -2.396768392486662f);
    pSH[32] = fTmpA * fC1;
    pSH[28] = fTmpA * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpA = 0.5900435899266435f;
    pSH[15] = fTmpA * fC0;
    pSH[9] = fTmpA * fS0;
    fTmpB = 1.770130769779931f * fZ;
    pSH[23] = fTmpB * fC0;
    pSH[17] = fTmpB * fS0;
    fTmpC = 4.403144694917254f * fZ2 + -0.4892382994352505f;
    pSH[33] = fTmpC * fC0;
    pSH[27] = fTmpC * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.6258357354491763f;
    pSH[24] = fTmpA * fC1;
    pSH[16] = fTmpA * fS1;
    fTmpB = 2.075662314881041f * fZ;
    pSH[34] = fTmpB * fC1;
    pSH[26] = fTmpB * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpC = 0.6563820568401703f;
    pSH[35] = fTmpC * fC0;
    pSH[25] = fTmpC * fS0;
}

// order 6
void SHEval6(const float fX, const float fY, const float fZ, float* pSH) {
    float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
    float fZ2 = fZ * fZ;

    pSH[0] = 0.2820947917738781f;
    pSH[2] = 0.4886025119029199f * fZ;
    pSH[6] = 0.9461746957575601f * fZ2 + -0.31539156525252f;
    pSH[12] = fZ * (1.865881662950577f * fZ2 + -1.119528997770346f);
    pSH[20] = 1.984313483298443f * fZ * pSH[12] + -1.006230589874905f * pSH[6];
    pSH[30] = 1.98997487421324f * fZ * pSH[20] + -1.002853072844814f * pSH[12];
    pSH[42] = 1.993043457183566f * fZ * pSH[30] + -1.001542020962219f * pSH[20];
    fC0 = fX;
    fS0 = fY;

    fTmpA = 0.48860251190292f;
    pSH[3] = fTmpA * fC0;
    pSH[1] = fTmpA * fS0;
    fTmpB = 1.092548430592079f * fZ;
    pSH[7] = fTmpB * fC0;
    pSH[5] = fTmpB * fS0;
    fTmpC = 2.285228997322329f * fZ2 + -0.4570457994644658f;
    pSH[13] = fTmpC * fC0;
    pSH[11] = fTmpC * fS0;
    fTmpA = fZ * (4.683325804901025f * fZ2 + -2.007139630671868f);
    pSH[21] = fTmpA * fC0;
    pSH[19] = fTmpA * fS0;
    fTmpB = 2.03100960115899f * fZ * fTmpA + -0.991031208965115f * fTmpC;
    pSH[31] = fTmpB * fC0;
    pSH[29] = fTmpB * fS0;
    fTmpC = 2.021314989237028f * fZ * fTmpB + -0.9952267030562385f * fTmpA;
    pSH[43] = fTmpC * fC0;
    pSH[41] = fTmpC * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.5462742152960395f;
    pSH[8] = fTmpA * fC1;
    pSH[4] = fTmpA * fS1;
    fTmpB = 1.445305721320277f * fZ;
    pSH[14] = fTmpB * fC1;
    pSH[10] = fTmpB * fS1;
    fTmpC = 3.31161143515146f * fZ2 + -0.47308734787878f;
    pSH[22] = fTmpC * fC1;
    pSH[18] = fTmpC * fS1;
    fTmpA = fZ * (7.190305177459987f * fZ2 + -2.396768392486662f);
    pSH[32] = fTmpA * fC1;
    pSH[28] = fTmpA * fS1;
    fTmpB = 2.11394181566097f * fZ * fTmpA + -0.9736101204623268f * fTmpC;
    pSH[44] = fTmpB * fC1;
    pSH[40] = fTmpB * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpA = 0.5900435899266435f;
    pSH[15] = fTmpA * fC0;
    pSH[9] = fTmpA * fS0;
    fTmpB = 1.770130769779931f * fZ;
    pSH[23] = fTmpB * fC0;
    pSH[17] = fTmpB * fS0;
    fTmpC = 4.403144694917254f * fZ2 + -0.4892382994352505f;
    pSH[33] = fTmpC * fC0;
    pSH[27] = fTmpC * fS0;
    fTmpA = fZ * (10.13325785466416f * fZ2 + -2.763615778544771f);
    pSH[45] = fTmpA * fC0;
    pSH[39] = fTmpA * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.6258357354491763f;
    pSH[24] = fTmpA * fC1;
    pSH[16] = fTmpA * fS1;
    fTmpB = 2.075662314881041f * fZ;
    pSH[34] = fTmpB * fC1;
    pSH[26] = fTmpB * fS1;
    fTmpC = 5.550213908015966f * fZ2 + -0.5045649007287241f;
    pSH[46] = fTmpC * fC1;
    pSH[38] = fTmpC * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpA = 0.6563820568401703f;
    pSH[35] = fTmpA * fC0;
    pSH[25] = fTmpA * fS0;
    fTmpB = 2.366619162231753f * fZ;
    pSH[47] = fTmpB * fC0;
    pSH[37] = fTmpB * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpC = 0.6831841051919144f;
    pSH[48] = fTmpC * fC1;
    pSH[36] = fTmpC * fS1;
}

// order 7
void SHEval7(const float fX, const float fY, const float fZ, float* pSH) {
    float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
    float fZ2 = fZ * fZ;

    pSH[0] = 0.2820947917738781f;
    pSH[2] = 0.4886025119029199f * fZ;
    pSH[6] = 0.9461746957575601f * fZ2 + -0.31539156525252f;
    pSH[12] = fZ * (1.865881662950577f * fZ2 + -1.119528997770346f);
    pSH[20] = 1.984313483298443f * fZ * pSH[12] + -1.006230589874905f * pSH[6];
    pSH[30] = 1.98997487421324f * fZ * pSH[20] + -1.002853072844814f * pSH[12];
    pSH[42] = 1.993043457183566f * fZ * pSH[30] + -1.001542020962219f * pSH[20];
    pSH[56] = 1.994891434824135f * fZ * pSH[42] + -1.000927213921958f * pSH[30];
    fC0 = fX;
    fS0 = fY;

    fTmpA = 0.48860251190292f;
    pSH[3] = fTmpA * fC0;
    pSH[1] = fTmpA * fS0;
    fTmpB = 1.092548430592079f * fZ;
    pSH[7] = fTmpB * fC0;
    pSH[5] = fTmpB * fS0;
    fTmpC = 2.285228997322329f * fZ2 + -0.4570457994644658f;
    pSH[13] = fTmpC * fC0;
    pSH[11] = fTmpC * fS0;
    fTmpA = fZ * (4.683325804901025f * fZ2 + -2.007139630671868f);
    pSH[21] = fTmpA * fC0;
    pSH[19] = fTmpA * fS0;
    fTmpB = 2.03100960115899f * fZ * fTmpA + -0.991031208965115f * fTmpC;
    pSH[31] = fTmpB * fC0;
    pSH[29] = fTmpB * fS0;
    fTmpC = 2.021314989237028f * fZ * fTmpB + -0.9952267030562385f * fTmpA;
    pSH[43] = fTmpC * fC0;
    pSH[41] = fTmpC * fS0;
    fTmpA = 2.015564437074638f * fZ * fTmpC + -0.9971550440218319f * fTmpB;
    pSH[57] = fTmpA * fC0;
    pSH[55] = fTmpA * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.5462742152960395f;
    pSH[8] = fTmpA * fC1;
    pSH[4] = fTmpA * fS1;
    fTmpB = 1.445305721320277f * fZ;
    pSH[14] = fTmpB * fC1;
    pSH[10] = fTmpB * fS1;
    fTmpC = 3.31161143515146f * fZ2 + -0.47308734787878f;
    pSH[22] = fTmpC * fC1;
    pSH[18] = fTmpC * fS1;
    fTmpA = fZ * (7.190305177459987f * fZ2 + -2.396768392486662f);
    pSH[32] = fTmpA * fC1;
    pSH[28] = fTmpA * fS1;
    fTmpB = 2.11394181566097f * fZ * fTmpA + -0.9736101204623268f * fTmpC;
    pSH[44] = fTmpB * fC1;
    pSH[40] = fTmpB * fS1;
    fTmpC = 2.081665999466133f * fZ * fTmpB + -0.9847319278346618f * fTmpA;
    pSH[58] = fTmpC * fC1;
    pSH[54] = fTmpC * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpA = 0.5900435899266435f;
    pSH[15] = fTmpA * fC0;
    pSH[9] = fTmpA * fS0;
    fTmpB = 1.770130769779931f * fZ;
    pSH[23] = fTmpB * fC0;
    pSH[17] = fTmpB * fS0;
    fTmpC = 4.403144694917254f * fZ2 + -0.4892382994352505f;
    pSH[33] = fTmpC * fC0;
    pSH[27] = fTmpC * fS0;
    fTmpA = fZ * (10.13325785466416f * fZ2 + -2.763615778544771f);
    pSH[45] = fTmpA * fC0;
    pSH[39] = fTmpA * fS0;
    fTmpB = 2.207940216581961f * fZ * fTmpA + -0.959403223600247f * fTmpC;
    pSH[59] = fTmpB * fC0;
    pSH[53] = fTmpB * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.6258357354491763f;
    pSH[24] = fTmpA * fC1;
    pSH[16] = fTmpA * fS1;
    fTmpB = 2.075662314881041f * fZ;
    pSH[34] = fTmpB * fC1;
    pSH[26] = fTmpB * fS1;
    fTmpC = 5.550213908015966f * fZ2 + -0.5045649007287241f;
    pSH[46] = fTmpC * fC1;
    pSH[38] = fTmpC * fS1;
    fTmpA = fZ * (13.49180504672677f * fZ2 + -3.113493472321562f);
    pSH[60] = fTmpA * fC1;
    pSH[52] = fTmpA * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpA = 0.6563820568401703f;
    pSH[35] = fTmpA * fC0;
    pSH[25] = fTmpA * fS0;
    fTmpB = 2.366619162231753f * fZ;
    pSH[47] = fTmpB * fC0;
    pSH[37] = fTmpB * fS0;
    fTmpC = 6.745902523363385f * fZ2 + -0.5189155787202604f;
    pSH[61] = fTmpC * fC0;
    pSH[51] = fTmpC * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.6831841051919144f;
    pSH[48] = fTmpA * fC1;
    pSH[36] = fTmpA * fS1;
    fTmpB = 2.6459606618019f * fZ;
    pSH[62] = fTmpB * fC1;
    pSH[50] = fTmpB * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpC = 0.7071627325245963f;
    pSH[63] = fTmpC * fC0;
    pSH[49] = fTmpC * fS0;
}

// order 8
void SHEval8(const float fX, const float fY, const float fZ, float* pSH) {
    float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
    float fZ2 = fZ * fZ;

    pSH[0] = 0.2820947917738781f;
    pSH[2] = 0.4886025119029199f * fZ;
    pSH[6] = 0.9461746957575601f * fZ2 + -0.31539156525252f;
    pSH[12] = fZ * (1.865881662950577f * fZ2 + -1.119528997770346f);
    pSH[20] = 1.984313483298443f * fZ * pSH[12] + -1.006230589874905f * pSH[6];
    pSH[30] = 1.98997487421324f * fZ * pSH[20] + -1.002853072844814f * pSH[12];
    pSH[42] = 1.993043457183566f * fZ * pSH[30] + -1.001542020962219f * pSH[20];
    pSH[56] = 1.994891434824135f * fZ * pSH[42] + -1.000927213921958f * pSH[30];
    pSH[72] = 1.996089927833914f * fZ * pSH[56] + -1.000600781069515f * pSH[42];
    fC0 = fX;
    fS0 = fY;

    fTmpA = 0.48860251190292f;
    pSH[3] = fTmpA * fC0;
    pSH[1] = fTmpA * fS0;
    fTmpB = 1.092548430592079f * fZ;
    pSH[7] = fTmpB * fC0;
    pSH[5] = fTmpB * fS0;
    fTmpC = 2.285228997322329f * fZ2 + -0.4570457994644658f;
    pSH[13] = fTmpC * fC0;
    pSH[11] = fTmpC * fS0;
    fTmpA = fZ * (4.683325804901025f * fZ2 + -2.007139630671868f);
    pSH[21] = fTmpA * fC0;
    pSH[19] = fTmpA * fS0;
    fTmpB = 2.03100960115899f * fZ * fTmpA + -0.991031208965115f * fTmpC;
    pSH[31] = fTmpB * fC0;
    pSH[29] = fTmpB * fS0;
    fTmpC = 2.021314989237028f * fZ * fTmpB + -0.9952267030562385f * fTmpA;
    pSH[43] = fTmpC * fC0;
    pSH[41] = fTmpC * fS0;
    fTmpA = 2.015564437074638f * fZ * fTmpC + -0.9971550440218319f * fTmpB;
    pSH[57] = fTmpA * fC0;
    pSH[55] = fTmpA * fS0;
    fTmpB = 2.011869540407391f * fZ * fTmpA + -0.9981668178901745f * fTmpC;
    pSH[73] = fTmpB * fC0;
    pSH[71] = fTmpB * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.5462742152960395f;
    pSH[8] = fTmpA * fC1;
    pSH[4] = fTmpA * fS1;
    fTmpB = 1.445305721320277f * fZ;
    pSH[14] = fTmpB * fC1;
    pSH[10] = fTmpB * fS1;
    fTmpC = 3.31161143515146f * fZ2 + -0.47308734787878f;
    pSH[22] = fTmpC * fC1;
    pSH[18] = fTmpC * fS1;
    fTmpA = fZ * (7.190305177459987f * fZ2 + -2.396768392486662f);
    pSH[32] = fTmpA * fC1;
    pSH[28] = fTmpA * fS1;
    fTmpB = 2.11394181566097f * fZ * fTmpA + -0.9736101204623268f * fTmpC;
    pSH[44] = fTmpB * fC1;
    pSH[40] = fTmpB * fS1;
    fTmpC = 2.081665999466133f * fZ * fTmpB + -0.9847319278346618f * fTmpA;
    pSH[58] = fTmpC * fC1;
    pSH[54] = fTmpC * fS1;
    fTmpA = 2.06155281280883f * fZ * fTmpC + -0.9903379376602873f * fTmpB;
    pSH[74] = fTmpA * fC1;
    pSH[70] = fTmpA * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpA = 0.5900435899266435f;
    pSH[15] = fTmpA * fC0;
    pSH[9] = fTmpA * fS0;
    fTmpB = 1.770130769779931f * fZ;
    pSH[23] = fTmpB * fC0;
    pSH[17] = fTmpB * fS0;
    fTmpC = 4.403144694917254f * fZ2 + -0.4892382994352505f;
    pSH[33] = fTmpC * fC0;
    pSH[27] = fTmpC * fS0;
    fTmpA = fZ * (10.13325785466416f * fZ2 + -2.763615778544771f);
    pSH[45] = fTmpA * fC0;
    pSH[39] = fTmpA * fS0;
    fTmpB = 2.207940216581961f * fZ * fTmpA + -0.959403223600247f * fTmpC;
    pSH[59] = fTmpB * fC0;
    pSH[53] = fTmpB * fS0;
    fTmpC = 2.15322168769582f * fZ * fTmpB + -0.9752173865600178f * fTmpA;
    pSH[75] = fTmpC * fC0;
    pSH[69] = fTmpC * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.6258357354491763f;
    pSH[24] = fTmpA * fC1;
    pSH[16] = fTmpA * fS1;
    fTmpB = 2.075662314881041f * fZ;
    pSH[34] = fTmpB * fC1;
    pSH[26] = fTmpB * fS1;
    fTmpC = 5.550213908015966f * fZ2 + -0.5045649007287241f;
    pSH[46] = fTmpC * fC1;
    pSH[38] = fTmpC * fS1;
    fTmpA = fZ * (13.49180504672677f * fZ2 + -3.113493472321562f);
    pSH[60] = fTmpA * fC1;
    pSH[52] = fTmpA * fS1;
    fTmpB = 2.304886114323221f * fZ * fTmpA + -0.9481763873554654f * fTmpC;
    pSH[76] = fTmpB * fC1;
    pSH[68] = fTmpB * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpA = 0.6563820568401703f;
    pSH[35] = fTmpA * fC0;
    pSH[25] = fTmpA * fS0;
    fTmpB = 2.366619162231753f * fZ;
    pSH[47] = fTmpB * fC0;
    pSH[37] = fTmpB * fS0;
    fTmpC = 6.745902523363385f * fZ2 + -0.5189155787202604f;
    pSH[61] = fTmpC * fC0;
    pSH[51] = fTmpC * fS0;
    fTmpA = fZ * (17.24955311049054f * fZ2 + -3.449910622098108f);
    pSH[77] = fTmpA * fC0;
    pSH[67] = fTmpA * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpA = 0.6831841051919144f;
    pSH[48] = fTmpA * fC1;
    pSH[36] = fTmpA * fS1;
    fTmpB = 2.6459606618019f * fZ;
    pSH[62] = fTmpB * fC1;
    pSH[50] = fTmpB * fS1;
    fTmpC = 7.984991490893139f * fZ2 + -0.5323327660595426f;
    pSH[78] = fTmpC * fC1;
    pSH[66] = fTmpC * fS1;
    fC0 = fX * fC1 - fY * fS1;
    fS0 = fX * fS1 + fY * fC1;

    fTmpA = 0.7071627325245963f;
    pSH[63] = fTmpA * fC0;
    pSH[49] = fTmpA * fS0;
    fTmpB = 2.91570664069932f * fZ;
    pSH[79] = fTmpB * fC0;
    pSH[65] = fTmpB * fS0;
    fC1 = fX * fC0 - fY * fS0;
    fS1 = fX * fS0 + fY * fC0;

    fTmpC = 0.72892666017483f;
    pSH[80] = fTmpC * fC1;
    pSH[64] = fTmpC * fS1;
}

}  // namespace Sloan2013SH
}  // namespace avs3renderer
