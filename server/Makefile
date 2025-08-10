all: remoteprog-server

include ../protobuf/protobuf.mk

OBJ = main.o server.o firmware.o runner.o llcomm.o ui.o messages.pb.o ssd1306-driver/ssd1306.o
CPPFLAGS = -Wall -Wextra -Wno-comment -I. -I../protobuf `pkg-config --cflags protobuf zlib`
CXXFLAGS = -std=c++20 
LDFLAGS = `pkg-config --libs protobuf zlib` -pthread -lpigpio

# CPPFLAGS += -DSIMULATE_LL

ifdef DEBUG
	CPPFLAGS += -ggdb -O0
else
	CPPFLAGS += -O3
endif

main.o llcomm.o server.o firmware.o: messages.pb.o

remoteprog-server: ${OBJ}
	g++ -o $@ $^ ${LDFLAGS}

clean: protobuf-clean
	rm -f remoteprog-server *.o ${OBJ}

install: remoteprog-server
	cp -f remoteprog-server /usr/local/bin/
	cp -f extra/remoteprog.service /etc/systemd/system/
	mkdir -p /etc/remoteprog
	cp -f extra/avrdude.conf /etc/remoteprog/
	cp -f extra/raspberrypi-swd.cfg /etc/remoteprog/
