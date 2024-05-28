#!/bin/sh
dstdir=../../bin

if [ -d ${dstdir} ]; then
	rm -rf ${dstdir}
fi;

mkdir -p ${dstdir}