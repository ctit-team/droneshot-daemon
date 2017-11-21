.PHONY: all clean emulation rpi

# common variables

OUTPUT = droneshot-daemon
CPPFLAGS =
LDLIBS = -lstdc++

# sources

SOURCES = droneshot-daemon.cpp \
          exception/formatter.cpp \
          hardware/hardware_interface.cpp \
          rpc/rpc_server.cpp

# phony rules

all: rpi

clean:
	-rm $(OUTPUT)
	-find . -name '*.o' -delete

emulation: CPPFLAGS += -DDS_HW_EMULATION
emulation: $(OUTPUT)

rpi: LDLIBS += -lbcm2835
rpi: $(OUTPUT)

# file rules

ifeq ($(MAKECMDGOALS),emulation)
$(OUTPUT): $(SOURCES:.cpp=.o) \
           hardware/emulation/hardware_interface.o
else
$(OUTPUT): $(SOURCES:.cpp=.o) \
           hardware/rpi/hardware_interface.o
endif
