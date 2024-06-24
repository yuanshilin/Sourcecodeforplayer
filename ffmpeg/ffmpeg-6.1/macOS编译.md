# macOS 编译 ffmpeg + 支持 audio vivid

### 调音库安装：

源码路径：http://172.28.10.84/caraudio/player/sourcecodeforplayer/-/tree/master/AudioFilter

执行 makefile/mac/installFilter.sh，需要 sudo 权限

### ffmpeg 编译命令：

```shell
./configure  --extra-libs="-lpthread -lm -ldhfilter " \
     --extra-ldflags='-ldl -L /usr/local/lib/' --ld="g++" --enable-cross-compile \
     --enable-gpl  --enable-libarcdav3a --enable-shared  --disable-static --enable-sdl \
     --extra-cflags=' -I /usr/local/include/FilterEngine '  --prefix=/usr/local/ffmpeg

make -j8

make install
```

### 碰到过的问题：

#### 1、make 报错

```
ld: -current_version: malformed version number '-compatibility_version' cannot fit in 32-bit xxxx.yy.zz

clang: error: linker command failed with exit code 1 (use -v to see invocation)
```

解决方法：

```
chmod +x ffbuild/*.sh
```

#### 2、执行`./ffplay xxx.mp4` 报错：

```
`dyld[48982]: Library not loaded: @rpath/libSDL2-2.0.0.dylib`

 `Referenced from: <149988C3-A055-3C27-A4F4-C4BA49BF1875> /Users/develop/Documents/workspace/SVN/c524/ffmpeg-6.1/ffplay`

 `Reason: no LC_RPATH's found`
```

##### 解决方法：

```
install_name_tool -add_rpath /usr/local/lib ffplay
```

### 3、缺少依赖库: libav3a_binaural_render.so libAVS3AudioDec.so model.bin

##### 解决方法：

```
libav3a_binaural_render.so libAVS3AudioDec.so放到/usr/local/lib目录下，model.bin放到ffmpeg目录下
```
