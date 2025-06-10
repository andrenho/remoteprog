all: remoteprog

include ../protobuf/protobuf.mk

OBJ = main.o client.o messages.pb.o
CPPFLAGS = -Wall -Wextra -std=c++20 `pkg-config --cflags protobuf`
LDFLAGS = `pkg-config --libs protobuf`

ifndef RELEASE
	CFLAGS += -ggdb -O0
endif

remoteprog: ${OBJ}
	g++ -o $@ $^ ${LDFLAGS}

clean: protobuf-clean
	rm -f remoteprog *.o ${OBJ}