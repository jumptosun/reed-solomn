CXX=g++
RS_SOURCE_ROOT=../..

default: demo.cpp ${RS_SOURCE_ROOT}/lib/libvcrs.a ${RS_SOURCE_ROOT}/include/reed_solomon.hpp
	${CXX} -o demo demo.cpp -I${RS_SOURCE_ROOT}/include -L${RS_SOURCE_ROOT}/lib -lvcrs

clean: 
	rm -rf demo
