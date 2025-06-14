CPPFLAGS=-Wall -Wextra
LDFLAGS=-pthread -lpigpio

test: ui.o
	g++ -o $@ $^ $(LDFLAGS)

clean:
	-rm -f ui.o test
