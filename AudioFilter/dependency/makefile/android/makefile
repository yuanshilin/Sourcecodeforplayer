
NDK=/Applications/AndroidNDK11579264.app/Contents/NDK
TOOLCHAIN=$(NDK)/toolchains/llvm/prebuilt/darwin-x86_64

API:=31
ifeq ($(APP_ABI), arm64-v8a)
CC=$(TOOLCHAIN)/bin/aarch64-linux-android$(API)-clang
CXX=$(TOOLCHAIN)/bin/aarch64-linux-android$(API)-clang++
else
CC=$(TOOLCHAIN)/bin/armv7a-linux-androideabi$(API)-clang
CXX=$(TOOLCHAIN)/bin/armv7a-linux-androideabi$(API)-clang++
endif
SRC_DIRS=../../src/cjson
SRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.c))
TARGET=../../lib/android/$(APP_ABI)/libcjson.so
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
