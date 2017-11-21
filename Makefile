.PHONY: all clean

all: bin
	$(MAKE) -C src
	cp src/droneshot-daemon/droneshot-daemon bin/

clean:
	$(MAKE) -C src clean
	-@rm bin/*

bin:
	mkdir bin
