#!/bin/bash

ANDROID_NDK=/Applications/AndroidNDK11579264.app/Contents/NDK

ReleaseVersion="V1.0"
cd ../../

WORKSPACE=$(cd "$dirname "$0")";pwd)
REVISION=$(git log --format="%h" -n 1 )
BUILD_ID=`date +%Y%m%d`
APP_ABIs="arm64-v8a armeabi-v7a"

#modify revision
cd $WORKSPACE/src/dhfilter
echo "//this cpp define a null function for audiofilter version info." > Filter_Version.cpp
echo -n "const char* ARCVIDEO_AUDIOFILTER_LIB_VERSION = \"" >> Filter_Version.cpp
echo -n ${REVISION} >> Filter_Version.cpp
echo "\";" >> Filter_Version.cpp

mkdir -p $WORKSPACE/delivery
mkdir -p $WORKSPACE/delivery/Filter
mkdir -p $WORKSPACE/delivery/Filter/inc
mkdir -p $WORKSPACE/delivery/Filter/doc
mkdir -p $WORKSPACE/delivery/Filter/lib
mkdir -p $WORKSPACE/delivery/Filter/lib/Android

for APP_ABI in ${APP_ABIs}
do

cd $WORKSPACE
mkdir android_build_${APP_ABI} && cd android_build_${APP_ABI}
mkdir -p $WORKSPACE/delivery/Filter/lib/Android/${APP_ABI}

cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
 -DANDROID_ABI=${APP_ABI} \
 -DANDROID_PLATFORM=android-31 \
 -DCMAKE_SYSTEM_NAME=Android \
 -DCMAKE_BUILD_TYPE=RELEASE \
 -DTARGET_OS_NAME=Android \
 ../
make 
cp -f $WORKSPACE/build/Android/${APP_ABI}/libdhfilter.so $WORKSPACE/delivery/Filter/lib/Android/${APP_ABI}/
rm -rf $WORKSPACE/android_build_${APP_ABI}
done

cp -f $WORKSPACE/src/dhfilter/FilterEngine.h $WORKSPACE/delivery/Filter/inc
cp -f $WORKSPACE/src/dhfilter/FilterTypes.h $WORKSPACE/delivery/Filter/inc
cp -f $WORKSPACE/doc/*.md $WORKSPACE/delivery/Filter/doc
cp -f $WORKSPACE/doc/*.jpg $WORKSPACE/delivery/Filter/doc

SDKName=ArcVideo_AudioFilter_SDK_Android@${ReleaseVersion}.${REVISION}.${BUILD_ID}.zip
cd $WORKSPACE/delivery/Filter
zip -r $SDKName ./*

cd $WORKSPACE/delivery
cp -rf ./Filter/*.zip ./
rm -rf ./Filter
rm -rf $WORKSPACE/build
