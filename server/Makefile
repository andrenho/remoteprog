all: remoteprog-server

include ../protobuf/protobuf.mk

OBJ = main.o server.o firmware.o runner.o messages.pb.o
CPPFLAGS = -Wall -Wextra -std=c++20 `pkg-config --cflags protobuf zlib`
LDFLAGS = `pkg-config --libs protobuf zlib`

ifndef RELEASE
	CPPFLAGS += -ggdb -O0
endif

server.o: messages.pb.o

remoteprog-server: ${OBJ}
	g++ -o $@ $^ ${LDFLAGS}

clean: protobuf-clean
	rm -f remoteprog-server *.o ${OBJ}