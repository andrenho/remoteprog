include ../protobuf/protobuf.mk

all: remoteprog-server

OBJ = main.o server.o messages.pb.o
CPPFLAGS = -Wall -Wextra -std=c++20 `pkg-config --cflags protobuf`
LDFLAGS = `pkg-config --libs protobuf`

ifndef RELEASE
	CFLAGS += -ggdb -O0
endif

remoteprog-server: ${OBJ}
	g++ -o $@ $^ ${LDFLAGS}

clean: protobuf-clean
	rm -f remoteprog-server *.o ${OBJ}