###########################################  
#Makefile for simple programs  
###########################################  

SRC:=$(wildcard ./src/*.cpp)
SRC+=$(wildcard ./src/*.hpp)

BUILD=./build
LIB=./lib

default:${SRC}
	@echo "build library and copy to lib"  
	if [ ! -d  ./build ]; then mkdir ./build; fi
	(cd build && cmake .. && make)
	if [ ! -d  ./lib ]; then mkdir ./lib; fi
	cp ./build/libvcrs.a ./lib
	cp ./src/reed_solomon.hpp ./include

  
.PRONY:clean  
clean:  
	rm -rf ${BUILD} ${LIB}
