CPPFLAGS=-Wall -Wextra -I.
LDFLAGS=-pthread -lpigpio

all: test

test: ui.o
	g++ -o $@ $^ $(LDFLAGS)

clean:
	-rm -f ui.o test
