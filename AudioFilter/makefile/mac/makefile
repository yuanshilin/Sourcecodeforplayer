CC:=gcc
CXX:=g++
SRC_DIRS=../../src/dhfilter
SRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.c))
SRC_CXX=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.cpp))

TARGET=/usr/local/lib/libdhfilter.a
OBJ:=$(SRC:.c=.o) $(SRC_CXX:.cpp=.o)

INC_EXTERN=-I../../src/dhfilter -I../../dependency/src/cjson
LIB_EXTERN=-L/usr/local/lib/
INCLUDE=$(INC_EXTERN)
DEP_LIB=-lcjson   $(LIB_EXTERN)
all: $(TARGET)

$(TARGET):$(OBJ)
	$(CXX) -shared -Wall -Werror,-Wsign-compare -Wpointer-arith -g -o  $(TARGET) $(OBJ) $(DEP_LIB) ;
	
%.o:%.c
	$(CC) -DDEBUG -Wall -g -fPIC -std=c99 $(INCLUDE) -o $@ -c $<
%.o:%.cpp
	$(CXX) -DDEBUG -Wall -g -fPIC -std=c++11 $(INCLUDE) -o $@ -c $<	
.PHONY:clean
clean:
	-rm -f $(OBJ) $(TARGET)
