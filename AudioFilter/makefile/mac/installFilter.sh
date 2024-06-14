#!/bin/sh

make clean
make

sudo cp -rf ../../src/dhfilter/FilterEngine.h /usr/local/include/FilterEngine/
#sudo cp -rf ../../output/mac-m2/libdhfilter.a /usr/local/lib/
