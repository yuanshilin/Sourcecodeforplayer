cmake_minimum_required(VERSION 3.1)
project(AudioFilter)

set(CMAKE_CXX_STANDARD 11)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/dependency/src/cjson
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dhfilter
)


#file(COPY  "${CMAKE_CURRENT_SOURCE_DIR}/controller.h"
#        "${CMAKE_CURRENT_SOURCE_DIR}/color.h"
#        ${CMAKE_CURRENT_SOURCE_DIR}/parser.h
#        DESTINATION  "${CMAKE_CURRENT_SOURCE_DIR}/../dist/include/light"  )

#string(TOUPPER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME)
message(${CMAKE_SYSTEM_NAME})
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(lib_output_path ${CMAKE_CURRENT_SOURCE_DIR}/build/${TARGET_OS_NAME})
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    add_definitions("-Wc++17-extensions")
    add_definitions("-Wno-invalid-source-encoding")
    add_definitions("-fdata-sections -ffunction-sections")
    # set(CMAKE_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX13.3.sdk")
    # set(CMAKE_CXX_FLAGS "--sysroot ${CMAKE_OSX_SYSROOT} ${CMAKE_CXX_FLAGS}")

    set(lib_output_path ${CMAKE_CURRENT_SOURCE_DIR}/build/${TARGET_OS_NAME})
elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
    add_definitions("-Wc++17-extensions")
    set(lib_output_path ${CMAKE_CURRENT_SOURCE_DIR}/build/${TARGET_OS_NAME}/${ANDROID_ABI})
else ()
    message(FATAL_ERROR "unknown CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
endif ()


# src
set(src ${CMAKE_CURRENT_SOURCE_DIR}/src/dhfilter)
set(sources ${src})

foreach (iter ${sources})
    aux_source_directory(${iter} lib_src)
endforeach ()

# dependency
set(src ${CMAKE_CURRENT_SOURCE_DIR}/dependency/src/cjson)
set(sources ${src})
foreach (iter ${sources})
    aux_source_directory(${iter} lib_src)
endforeach ()

message(${lib_output_path})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${lib_output_path})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${lib_output_path})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${lib_output_path})

message(${lib_src})
#
add_library(dhfilter SHARED ${lib_src})

#target_link_libraries(light
#        ${CMAKE_CURRENT_SOURCE_DIR}/../input/libs/${TARGET_OS_NAME}/libbase.a
#        )
