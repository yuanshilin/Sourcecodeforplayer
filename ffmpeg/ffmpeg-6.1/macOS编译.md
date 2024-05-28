# macOS编译ffmpeg + 支持audio vivid

### 编译命令

./configure  --extra-libs="-lpthread -lm"  --extra-ldflags=-ldl  --ld="g++" --enable-gpl  --enable-libarcdav3a --enable-shared  --disable-static --enable-sdl --prefix=/usr/local/ffmpeg

make & make install

### 碰到过的问题：

#### 1、执行`./ffplay xxx.mp4` 报错：

```
`dyld[48982]: Library not loaded: @rpath/libSDL2-2.0.0.dylib`

 `Referenced from: <149988C3-A055-3C27-A4F4-C4BA49BF1875> /Users/develop/Documents/workspace/SVN/c524/ffmpeg-6.1/ffplay`

 `Reason: no LC_RPATH's found`
```

##### 解决方法：

```
install_name_tool -add_rpath /usr/local/lib ffplay
```



### 2、缺少依赖库: libav3a_binaural_render.so    libAVS3AudioDec.so  model.bin 

##### 解决方法：

```
libav3a_binaural_render.so libAVS3AudioDec.so放到/usr/local/bin目录下，model.bin放到ffmpeg目录下
```

