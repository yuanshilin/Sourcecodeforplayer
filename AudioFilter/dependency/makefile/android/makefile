
NDK=/Applications/android-ndk-r20b
TOOLCHAIN=$(NDK)/toolchains/llvm/prebuilt/darwin-x86_64
ARCH=arm64
CPU=armv8-a
API:=24
CC=$(TOOLCHAIN)/bin/aarch64-linux-android$(API)-clang
CXX=$(TOOLCHAIN)/bin/aarch64-linux-android$(API)-clang++

SRC_DIRS=../../src/cjson
SRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.c))
TARGET=../../lib/android/arm64-v8a/libcjson.so
OBJ:=$(SRC:.c=.o)

INC_EXTERN=-I../../src/cjson
  
INCLUDE=$(INC_EXTERN)

all: $(TARGET)
$(TARGET):$(OBJ)
	$(CC) -shared -Wall -g -o  $(TARGET) $(OBJ) $(DEP_LIB);
%.o:%.c
	$(CC) -DDEBUG -Wall -g -fPIC -std=c99 $(INCLUDE) -o $@ -c $<
	
.PHONY:clean
clean:
	-rm -f $(OBJ) $(TARGET)
