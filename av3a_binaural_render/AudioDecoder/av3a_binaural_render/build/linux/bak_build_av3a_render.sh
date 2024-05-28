#!/bin/bash

NDK=/mnt/data/local-disk1/home/sdh/trunk/autobuild/android-ndk-r20b-linux-x86_64/android-ndk-r20b/
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64

#需要在当前目录下新建temp文件夹
export TMPDIR=temp

function build_android {
echo "Compiling av3a_binaural_render for $CPU"
   
#make -f av3a_binaural_render_android.mk clean
make -f av3a_binaural_render_android.mk

echo "The Compilation of av3a_binaural_render for $CPU is completed"
}

#armv8-a
ARCH=arm64
CPU=armv8-a
API=24
CC=$TOOLCHAIN/bin/aarch64-linux-android$API-clang
CXX=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++
SYSROOT=$TOOLCHAIN/sysroot
CROSS=aarch64-linux-android
CROSS_PREFIX=$TOOLCHAIN/bin/$CROSS
PREFIX=/home/arcvideo/ljin/ffmpeg-6.1/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU"
#ADDI_LDFLAGS="-ldl -L../dependency/linux/arm" /home/arcvideo/ljin/depend_so
#ADDI_LDFLAGS="-ldl -L$X264_LIB"
build_android

#armv7-a
ARCH=arm

CPU=armv7-a
API=24
CC=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang
CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++
SYSROOT=$TOOLCHAIN/sysroot
CROSS=arm-linux-androideabi
CROSS_PREFIX=$TOOLCHAIN/bin/$CROSS
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU -mfloat-abi=softfp -mfpu=vfp -marm"
#ADDI_LDFLAGS="-ldl -L../dependency/linux/arm"
build_android

