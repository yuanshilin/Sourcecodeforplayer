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

#ifndef API_TYPES_H
#define API_TYPES_H

#include <stdbool.h>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_context audio_context;

typedef enum {
    ERROR = -1,
    SUCCESS = 0,
    SOURCE_NOT_FOUND = -1001,
    ILLEGAL_VALUE = -1005,
    CONTEXT_NOT_CREATED = -1006,
    CONTEXT_NOT_READY = -1007,
    CONTEXT_REPEATED_INITIALIZATION = -1008,
} result;

typedef enum {
    AMBISONIC_FIRST_ORDER = 1,
    AMBISONIC_SECOND_ORDER,
    AMBISONIC_THIRD_ORDER,
    AMBISONIC_FOURTH_ORDER,
    AMBISONIC_FIFTH_ORDER,
    AMBISONIC_SIXTH_ORDER,
    AMBISONIC_SEVENTH_ORDER,
} rendering_mode;

typedef enum {
    SOURCE_SPATIALIZE = 0,
    SOURCE_BYPASS = 1,
} source_mode;

typedef enum { SN3D, N3D } ambisonic_normalization_type;

#ifdef __cplusplus
}
#endif

#endif  // API_TYPES_H
