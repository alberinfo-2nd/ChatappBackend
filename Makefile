SRC_DIR=./src
INC_DIR=./include

CXXFLAGS=-I $(INC_DIR) -Wall -Wextra -Werror

CPP_SOURCES=""
RM_Command=""

ifeq ($(OS), Windows_NT)
	CPP_SOURCES=$(wildcard $(SRC_DIR)/*.cpp)
	CXXFLAGS+=-lws2_32 -o program.exe
	SHELL:=C:\WINDOWS\system32\cmd.exe
	RM_Command=@if exist program.exe del /q program.exe
else
	CPP_SOURCES=$(shell find $(SRC_DIR) -type f -name '*.cpp')
	CXXFLAGS+=-o program
	RM_Command=rm program
endif

build: clean .WAIT $(CPP_OBJS)
	g++ $(CPP_SOURCES) $(CXXFLAGS)

run:
	./program

all: build .WAIT run

clean:
	-$(RM_Command)