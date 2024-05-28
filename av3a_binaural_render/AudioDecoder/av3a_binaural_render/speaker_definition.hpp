
/* ==================================================================================================

The copyright in this software is being made available under the License included below.

Copyright (c) 2022 Beijing Zitiao Network Technology Co., Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions
  and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions
  and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the ITU/ISO/IEC nor the names of its contributors may be used to endorse or promote
  products derived from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE
IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

========================================================================================================*/

#ifndef AVS3_RENDER_SPEAKER_DEFINITION_H
#define AVS3_RENDER_SPEAKER_DEFINITION_H

#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>

namespace AVS3 {

enum class SpeakerLayout {
    Layout_Mono,
    Layout_Stereo,
    Layout_3_0,
    Layout_4_0,
    Layout_5_0,
    Layout_5_1,
    Layout_6_1,
    Layout_5_1_2,
    Layout_5_1_4,
    Layout_7_0,
    Layout_7_1,
    Layout_7_0_2,
    Layout_7_1_2,
    Layout_7_1_4,
    Layout_Unknown
};

struct Speaker {
    float azimuth{};
    float elevation{};
    float distance{1};
    std::string label{};
    std::string channelFormatID{};
    std::string channelName{};

    Speaker(float azimuth, float elevation, float distance, std::string label, std::string channelFormatID)
            : azimuth(azimuth), elevation(elevation), distance(distance), label(label), channelFormatID(channelFormatID) {
    }
};

struct SpeakerSetup {
    std::string name;
    std::string showName;
    std::string packFormatId;
    SpeakerLayout layout;
    std::vector<Speaker> speakers;

    SpeakerSetup(std::string name, std::string showName, std::string packFormatId,
                 SpeakerLayout layout, std::vector<Speaker> speakers)
            : name(name), showName(showName), layout(layout), speakers(speakers), packFormatId(packFormatId) {
    }
};


static const std::map<SpeakerLayout, SpeakerSetup> SPEAKER_Layout_2094 = {
        {
                SpeakerLayout::Layout_Mono,
                SpeakerSetup("Mono", "Mono", "AP_00010001", SpeakerLayout::Layout_Mono,
                             std::vector<Speaker>{
                                     Speaker{0.0f, 0.0f, 1.0f, "M", "AC_00010003"}
                             })
        },
        {
                SpeakerLayout::Layout_Stereo,
                SpeakerSetup("Stereo", "Stereo", "AP_00010002", SpeakerLayout::Layout_Stereo,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"}
                             })
        },
        {
                SpeakerLayout::Layout_3_0,
                SpeakerSetup("3.0", "3.0", "AP_0001000a", SpeakerLayout::Layout_3_0,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"}
                             })
        },
        {
                SpeakerLayout::Layout_4_0,
                SpeakerSetup("4.0", "4.0", "AP_0001000b", SpeakerLayout::Layout_4_0,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"},
                                     Speaker{180.0f, 0.0f, 1.0f, "BC", "AC_00010009"}
                             })
        },
        {
                SpeakerLayout::Layout_5_0,
                SpeakerSetup("5.0", "5.0", "AP_0001000c", SpeakerLayout::Layout_5_0,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"},
                                     Speaker{110.0f, 0.0f, 1.0f, "Ls", "AC_00010005"},
                                     Speaker{-110.0f, 0.0f, 1.0f, "Rs", "AC_00010006"}
                             })
        },
        {
                SpeakerLayout::Layout_5_1,
                SpeakerSetup("5.1", "5.1", "AP_00010003", SpeakerLayout::Layout_5_1,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"},
                                     Speaker{45.0f, -30.0f, 1.0f, "LFE", "AC_00010004"},
                                     Speaker{110.0f, 0.0f, 1.0f, "Ls", "AC_00010005"},
                                     Speaker{-110.0f, 0.0f, 1.0f, "Rs", "AC_00010006"}
                             })
        },
        {
                SpeakerLayout::Layout_6_1,
                SpeakerSetup("6.1", "6.1", "AP_0001000d", SpeakerLayout::Layout_6_1,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"},
                                     Speaker{45.0f, -30.0f, 1.0f, "LFE", "AC_00010004"},
                                     Speaker{110.0f, 0.0f, 1.0f, "Ls", "AC_00010005"},
                                     Speaker{-110.0f, 0.0f, 1.0f, "Rs", "AC_00010006"},
                                     Speaker{180.0f, 0.0f, 1.0f, "BC", "AC_00010009"}
                             })
        },
        {
                SpeakerLayout::Layout_7_1,
                SpeakerSetup("7.1", "7.1", "AP_0001000e", SpeakerLayout::Layout_7_1,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"},
                                     Speaker{45.0f, -30.0f, 1.0f, "LFE", "AC_00010004"},
                                     Speaker{110.0f, 0.0f, 1.0f, "Ls", "AC_00010005"},
                                     Speaker{-110.0f, 0.0f, 1.0f, "Rs", "AC_00010006"},
                                     Speaker{45.0f, 0.0f, 1.0f, "FLM", "AC_00010026"},
                                     Speaker{-45.0f, 0.0f, 1.0f, "FRM", "AC_00010027"}
                             })
        },
        {
                SpeakerLayout::Layout_5_1_2,
                SpeakerSetup("5.1.2", "5.1.2", "AP_00010004", SpeakerLayout::Layout_5_1_2,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"},
                                     Speaker{45.0f, -30.0f, 1.0f, "LFE", "AC_00010004"},
                                     Speaker{110.0f, 0.0f, 1.0f, "Ls", "AC_00010005"},
                                     Speaker{-110.0f, 0.0f, 1.0f, "Rs", "AC_00010006"},
                                     Speaker{30.0f, 45.0f, 1.0f, "Ltf", "AC_0001000d"},
                                     Speaker{-30.0f, 45.0f, 1.0f, "Rtf", "AC_0001000f"}
                             })
        },
        {
                SpeakerLayout::Layout_5_1_4,
                SpeakerSetup("5.1.4", "5.1.4", "AP_00010005", SpeakerLayout::Layout_5_1_4,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"},
                                     Speaker{0.0f, 0.0f, 1.0f, "LFE", "AC_00010004"},
                                     Speaker{110.0f, 0.0f, 1.0f, "Ls", "AC_00010005"},
                                     Speaker{-110.0f, 0.0f, 1.0f, "Rs", "AC_00010006"},
                                     Speaker{30.0f, 30.0f, 1.0f, "Ltf", "AC_0001000d"},
                                     Speaker{-30.0f, 30.0f, 1.0f, "Rtf", "AC_0001000f"},
                                     Speaker{110.0f, 30.0f, 1.0f, "Ltr", "AC_00010010"},
                                     Speaker{-110.0f, 30.0f, 1.0f, "Rtr", "AC_00010012"}
                             }),
        },
        {
                SpeakerLayout::Layout_7_1_2,
                SpeakerSetup("7.1.2", "7.1.2", "AP_00010016", SpeakerLayout::Layout_7_1_2,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"},
                                     Speaker{45.0f, -30.0f, 1.0f, "LFE", "AC_00010004"},
                                     Speaker{90.0f, 0.0f, 1.0f, "SiL", "AC_0001000a"},
                                     Speaker{-90.0f, 0.0f, 1.0f, "SiR", "AC_0001000b"},
                                     Speaker{135.0f, 0.0f, 1.0f, "LB", "AC_0001000c"},
                                     Speaker{-135.0f, 0.0f, 1.0f, "RB", "AC_0001000d"},
                                     Speaker{90.0f, 45.0f, 1.0f, "TpSiL", "AC_00010013"},
                                     Speaker{-90.0f, 45.0f, 1.0f, "TpSiR", "AC_00010014"}
                             })
        },
        {
                SpeakerLayout::Layout_7_1_4,
                SpeakerSetup("7.1.4", "7.1.4", "AP_00010017", SpeakerLayout::Layout_7_1_4,
                             std::vector<Speaker>{
                                     Speaker{30.0f, 0.0f, 1.0f, "L", "AC_00010001"},
                                     Speaker{-30.0f, 0.0f, 1.0f, "R", "AC_00010002"},
                                     Speaker{0.0f, 0.0f, 1.0f, "C", "AC_00010003"},
                                     Speaker{45.0f, -30.0f, 1.0f, "LFE", "AC_00010004"},
                                     Speaker{90.0f, 0.0f, 1.0f, "Lss", "AC_0001000a"},
                                     Speaker{-90.0f, 0.0f, 1.0f, "Rss", "AC_0001000b"},
                                     Speaker{135.0f, 45.0f, 1.0f, "Lrs", "AC_0001000c"},
                                     Speaker{-135.0f, 45.0f, 1.0f, "Rrs", "AC_0001000d"},
                                     Speaker{45.0f, 45.0f, 1.0f, "LH", "AC_00010022"},
                                     Speaker{-45.0f, 45.0f, 1.0f, "RH", "AC_00010023"},
                                     Speaker{135.0f, 45.0f, 1.0f, "Ltb", "AC_0001001e"},
                                     Speaker{-135.0f, 45.0f, 1.0f, "Rtb", "AC_0001001f"}
                             })
        },
};

inline std::string convertHex(int num, int size = 4) {
    int len = 1;
    for (int i = 1; i < num; ++i) {
        if (std::pow(16, i - 1) <= num && std::pow(16, i) > num) {
            len = i;
            break;
        }
    }
    std::string ret(len, '0');
    sprintf((char *) ret.c_str(), "%x", num);
    if (len < size) {
        std::reverse(ret.begin(), ret.end());
        while (len++ < size) {
            ret.push_back('0');
        }
        std::reverse(ret.begin(), ret.end());
    }
    return ret;
}

inline std::string getAudioPackID(int type, int index, bool defaultIndex = false) {
    if (defaultIndex) {
        return std::string("AP_000") + std::to_string(type) + convertHex(index);
    } else {
        return std::string("AP_000") + std::to_string(type) + convertHex((int) std::pow(16, 3) + index);
    }
}
}
#endif //AVS3_RENDER_SPEAKER_DEFINITION_H
