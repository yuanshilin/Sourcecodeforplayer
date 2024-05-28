#!/bin/sh
dstbase=../../../../../../Versions/BaseLibVersions/common/libosbase/linux/

if [ -d ${dstbase} ]; then
	rm -rf ${dstbase}
fi;

mkdir -p ${dstbase}include
mkdir -p ${dstbase}lib

cp -r ../../include/*.h ${dstbase}include/
cp -r ../../bin/* ${dstbase}lib
