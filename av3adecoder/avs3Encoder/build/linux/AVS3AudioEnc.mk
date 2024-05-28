###
###     Makefile for AVS3Encoder project
###
###             generated for LINUX environments
###             by shengyancong
###

NAME = libAVS3AudioEnc.so
OS_ARCH := $(shell uname -m)
### include debug information: 1=yes, 0=no
DBG?= 0
### Generate 32 bit executable : 1=yes, 0=no
M32?= 0
### include O level optimization : 0-3
OPT?= 0
### Static Compilation
STC?= 0
### Support AVX2
AVX2?= 0
### Support AVX512
AVX512?= 0
### Support NEON
NEON?= 0
### prof on ?
PROF?= 0

DEPEND= dependencies

ifeq ($(M32),1)
CC=     $(shell which gcc) -m32
else
CC=     $(shell which gcc)
endif

ifeq ($(STC),1)
ifeq ($(DBG),1)  ### Do not use static compilation for Debug mode
STC=0
STATIC=
else
STATIC= -static
endif
else
STATIC=
endif

SUFFIX=

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

CFLAGS += -Wl,--no-undefined -Wl,--retain-symbols-file=retain_symbols.txt -Wl,-version-script=version-script.txt

SRC_DIRS=../../src ../../../libavs3_common ../../../libavs3_debug
SRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.c)) 
TARGET=../../../bin/libAVS3AudioEnc.so
OBJ:=$(SRC:.c=.o)

LIB_EXTERN=-L../../lib/  -L../../deps/lib
INC_EXTERN=-I../../../libavs3_common/ -I../../../libavs3_debug

INCLUDE=$(INC_EXTERN) -I../../include
DEP_LIB=-lpthread -lm -ldl -lrt  $(LIB_EXTERN)

.PHONY: default distclean clean depend

default: messages depend bin

messages:
ifeq ($(M32),1)
	@echo 'Compiling with M32 support...'
endif
ifeq ($(DBG),1)
	@echo 'Compiling with Debug support...'
	@echo 'Note static compilation not supported in this mode.'
endif
ifeq ($(STC),1)
	@echo 'Compiling with -static support...'
endif

dependencies:
	@echo "" >dependencies

clean:
	@echo remove all objects
	@rm -rf $(OBJ)

distclean: clean
	@rm -f $(DEPEND) tags
	@rm -f $(TARGET)

bin:    $(OBJ)
	@echo
	@echo 'creating binary "$(TARGET)"'
	@$(CC) -shared -fPIC $(CFLAGS) -o $(TARGET) $(OBJ) $(DEP_LIB)

	@echo '... done'
	@echo

depend:
	@echo
	@echo 'checking dependencies'
	@$(SHELL) -ec '$(CC) $(CFLAGS) -MM $(INCLUDE) $(SRC)  \
         | sed '\''s@\(.*\)\.o[ :]@$(OBJ)/\1.o$(SUFFIX):@g'\''               \
         >$(DEPEND)'
	@echo

%.o:%.c
	$(CC) $(CFLAGS) -fPIC -std=c99 $(INCLUDE) -o $@ -c $<

-include $(DEPEND)