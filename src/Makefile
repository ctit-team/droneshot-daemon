.PHONY: all clean

all:
	$(MAKE) -C droneshot-api
	$(MAKE) -C droneshot-lib

	$(MAKE) -C droneshot-daemon

clean:
	$(MAKE) -C droneshot-daemon clean

	$(MAKE) -C droneshot-api clean
	$(MAKE) -C droneshot-lib clean
