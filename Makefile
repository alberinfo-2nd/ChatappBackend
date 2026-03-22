SRC_DIR=./src
INC_DIR=./include

CPP_SOURCES=$(shell find $(SRC_DIR) -type f -name '*.cpp')

CXXFLAGS=-I $(INC_DIR) -Wall -Wextra -Werror -o program

build: clean .WAIT $(CPP_OBJS)
	g++ $(CPP_SOURCES) $(CXXFLAGS)

run:
	./program

all: build .WAIT run

clean:
	-rm program