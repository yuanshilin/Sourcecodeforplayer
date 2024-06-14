CC:=gcc
SRC_DIRS=../../src/dhfilter
SRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.c))
TARGET=/usr/local/lib/libdhfilter.a
OBJ:=$(SRC:.c=.o)

INC_EXTERN=-I../../src/dhfilter -I../../dependency/src/cjson
LIB_EXTERN=-L/usr/local/lib/
INCLUDE=$(INC_EXTERN)
DEP_LIB=-lcjson   $(LIB_EXTERN)
all: $(TARGET)

$(TARGET):$(OBJ)
	$(CC) -shared -Wall -g -o  $(TARGET) $(OBJ) $(DEP_LIB);
	
%.o:%.c
	$(CC) -DDEBUG -Wall -g -fPIC -std=c99 $(INCLUDE) -o $@ -c $<
	
.PHONY:clean
clean:
	-rm -f $(OBJ) $(TARGET)
