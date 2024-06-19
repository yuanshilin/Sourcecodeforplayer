### 说明

#### 编译

##### OSX

- OSX Arm 平台编译 x86_64 平台的程序

```shell

cd ffmpeg/ffmpeg-6.1
./configure --enable-cross-compile --prefix=../dist/x86_64 --arch=x86_64 --cc='clang -arch x86_64'
make -j8
make install

# 查看编译结果
lipo -info ../dist/x86_64/lib/libavcodec.a

# 清理
make distclean
```
