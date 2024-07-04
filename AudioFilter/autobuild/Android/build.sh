#!/bin/sh

ReleaseVersion="V1.0"
cd ../../

WORKSPACE=$(cd "$dirname "$0")";pwd)
REVISION=$(git log --format="%h" -n 1 )
BUILD_ID=`date +%Y%m%d`
APP_ABIs="arm64-v8a armeabi-v7a"

mkdir -p $WORKSPACE/delivery
mkdir -p $WORKSPACE/delivery/Filter
mkdir -p $WORKSPACE/delivery/Filter/inc
mkdir -p $WORKSPACE/delivery/Filter/doc
mkdir -p $WORKSPACE/delivery/Filter/lib
mkdir -p $WORKSPACE/delivery/Filter/lib/Android

for APP_ABI in ${APP_ABIs}
do
mkdir -p $WORKSPACE/delivery/Filter/lib/Android/${APP_ABI}
cd $WORKSPACE/makefile/android
make -f makefile_so clean
make -f makefile_so APP_ABI=${APP_ABI}
cp -f $WORKSPACE/output/android/${APP_ABI}/libdhfilter.so $WORKSPACE/delivery/Filter/lib/Android/${APP_ABI}/
cp -f $WORKSPACE/dependency/lib/android/${APP_ABI}/libcjson.so $WORKSPACE/delivery/Filter/lib/Android/${APP_ABI}/
done

cp -f $WORKSPACE/src/dhfilter/FilterEngine.h $WORKSPACE/delivery/Filter/inc
cp -f $WORKSPACE/doc/*.md $WORKSPACE/delivery/Filter/doc
cp -f $WORKSPACE/doc/*.jpg $WORKSPACE/delivery/Filter/doc

SDKName=ArcVideo_AudioFilter_SDK_Android@${ReleaseVersion}.${REVISION}.${BUILD_ID}.zip
cd $WORKSPACE/delivery/Filter
zip -r $SDKName ./*

cd $WORKSPACE/delivery
cp -rf ./Filter/*.zip ./
rm -rf ./Filter
