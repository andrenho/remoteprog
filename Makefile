all:
	$(MAKE) -C client remoteprog
	$(MAKE) -C server remoteprog-server

clean:
	$(MAKE) -C client clean
	$(MAKE) -C server clean
