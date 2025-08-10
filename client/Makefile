all: remoteprog

include ../protobuf/protobuf.mk

OBJ = main.o client.o lastcall.o request.o messages.pb.o
CPPFLAGS = -Wall -Wextra -std=c++20 -I. -I../protobuf `pkg-config --cflags protobuf zlib`
LDFLAGS = `pkg-config --libs protobuf zlib`

ifdef RELEASE
	CPPFLAGS += -g -ggdb -O0
else
	CPPFLAGS += -O3
endif

main.o: messages.pb.o

client.o: messages.pb.o

remoteprog: ${OBJ}
	g++ -o $@ $^ ${LDFLAGS}

install: remoteprog
	cp -f remoteprog /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/remoteprog

clean: protobuf-clean
	rm -f remoteprog *.o ${OBJ}
