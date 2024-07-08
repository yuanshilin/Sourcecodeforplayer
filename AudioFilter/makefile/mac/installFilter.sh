#!/bin/sh

make clean
make

# 判断 FilterEngine 目录是否存在
if [ ! -d "/usr/local/include/FilterEngine" ]; then
    sudo mkdir /usr/local/include/FilterEngine
fi

sudo cp -rf ../../src/dhfilter/FilterEngine.h /usr/local/include/FilterEngine/
