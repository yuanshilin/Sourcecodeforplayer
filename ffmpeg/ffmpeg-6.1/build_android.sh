#!/bin/bash

NDK=/root/android-ndk-r20b
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64

#需要在当前目录下新建temp文件夹
#export TMPDIR=temp

function build_android {
echo "Compiling FFmpeg for $CPU"
./configure \
    --prefix=$PREFIX \
    --disable-doc \
    --disable-debug \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-avdevice \
    --disable-libx264 \
    --disable-static \
    --enable-shared \
    --enable-nonfree \
    --enable-gpl \
    --enable-pthreads \
    --enable-neon \
    --enable-hwaccels \
    --enable-jni \
    --enable-mediacodec \
    --enable-libarcdavs2 \
    --enable-libarcdavs3 \
    --enable-libarcdav3a \
    --enable-version3 \
    --enable-openssl \
    --enable-protocols \
    --enable-protocol=https \
    --enable-cross-compile \
    --cross-prefix=$CROSS_PREFIX- \
    --target-os=android \
    --arch=$ARCH \
    --cpu=$CPU \
    --cc=$CC \
    --cxx=$CXX \
    --sysroot=$SYSROOT \
    --extra-cflags="-O3 -fPIC -I../dependency/include/ $OPTIMIZE_CFLAGS" \
    --extra-ldflags="-ldl $ADDI_LDFLAGS  -lssl -lcrypto" \
    
make clean
make -j8
make install
echo "The Compilation of FFmpeg for $CPU is completed"
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
DEPEND_INCLUDE=/usr/local/include
DEPEND_LIB=/usr/local/lib
#打包成一个so库
#$CROSS_PREFIX-ld.bfd \
#-rpath-link=$SYSROOT/usr/lib/$CROSS/$API \
#-L$SYSROOT/usr/lib/$CROSS/$API \
#-L$PREFIX/lib \
#-L$SMBCLIENT/$CPU/lib \
#-soname libinkffmpeg.so -shared -nostdlib -Bsymbolic --whole-archive --no-undefined -o \
#$PREFIX/libinkffmpeg.so \
#    libavcodec/libavcodec.a \
#    libavformat/libavformat.a \
#    libavutil/libavutil.a \
#    libswresample/libswresample.a \
#    libswscale/libswscale.a \
#    -lc -lm -lz -ldl -llog --dynamic-linker=/system/bin/linker \
#    $TOOLCHAIN/lib/gcc/$CROSS/4.9.x/libgcc_real.a
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
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU"
#ADDI_LDFLAGS="-ldl -L../dependency/linux/arm" /home/arcvideo/ljin/depend_so
ADDI_LDFLAGS="-ldl -L../dependency/android/armv8-a/openssl"
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
ADDI_LDFLAGS="-ldl -L../dependency/android/armv7-a/openssl"
#build_android

