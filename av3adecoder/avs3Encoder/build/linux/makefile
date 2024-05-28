OS_ARCH := $(shell uname -m)
### include debug information: 1=yes, 0=no
DBG?= 0
### include O level optimization : 0-3
OPT?= 3
### Support AVX2
AVX2?= 1
### Support AVX512
AVX512?= 0
### Support NEON
NEON?= 0
### prof on ?
PROF?= 0

ifeq ($(OS_ARCH),x86_64)
	ifeq ($(AVX2),1)
		CFLAGS += -mavx2 -DSUPPORT_AVX2
	else
		ifeq ($(AVX512),1)
			CFLAGS += -mavx512f -mavx512pf -mavx512er -mavx512cd -mavx512vl -mavx512bw -mavx512dq -mavx512ifma -mavx512vbmi -DSUPPORT_AVX512
		else
			CFLAGS += 
		endif
	endif
endif

ifeq ($(OS_ARCH),aarch64)
	ifeq ($(NEON),1)
		CFLAGS += -fsigned-char -DARCH_AARCH64 -DSUPPORT_NEON
	else
		CFLAGS += -fsigned-char -DARCH_AARCH64
	endif
endif

ifeq ($(PROF),1)
  CFLAGS += -L/usr/local/lib -lprofiler
endif

OPT_FLAG = -O$(OPT)
ifeq ($(DBG),1)
	CFLAGS += -g -D_DEBUG -Wall
else
	CFLAGS += $(OPT_FLAG) -Wall
endif

CC:=gcc
SRC_DIRS=../../src ../../../libavs3_common ../../../libavs3_debug
SRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.c)) 
TARGET=../../../bin/avs3Encoder
OBJ:=$(SRC:.c=.o) 

LIB_EXTERN=-L../../lib/  -L../../deps/lib
INC_EXTERN=-I../../../libavs3_common/ -I../../../libavs3_debug
  
INCLUDE=$(INC_EXTERN) -I../../include
DEP_LIB=-lpthread -lm -ldl -lrt  $(LIB_EXTERN) 
all:prebuild $(TARGET)
prebuild:
#	chmod +x *.sh
#	./prebuild.sh
$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(DEP_LIB);
	
%.o:%.c
	$(CC) $(CFLAGS) -fPIC -std=c99 $(INCLUDE) -o $@ -c $<
	
.PHONY:clean
clean:
	-rm -f $(OBJ) $(TARGET)
