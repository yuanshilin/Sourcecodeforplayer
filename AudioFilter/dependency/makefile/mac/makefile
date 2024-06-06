CC:=gcc
SRC_DIRS=../../src/cjson
SRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.c))
TARGET=../../lib/mac-m2/libcjson.a
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
