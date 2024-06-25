### 说明

#### 编译

```shell
git clone http://172.28.10.84/caraudio/player/sourcecodeforplayer.git
```

##### OSX

- AudioFilter

```shell
cd AudioFilter/makefile/mac
make

chmod +x installFilterEngine.sh
./installFilterEngine.sh
```

- SDL

```shell
git clone http://172.28.10.84/caraudio/sdl.git

# 参考ReadMe.md

# 验证版本
lipo -info libSDL2.a
```

- ffmpeg

[《macOS 编译》](./ffmpeg/ffmpeg-6.1/macOS编译.md) osx 编译 ffmpeg，以及错误解决
