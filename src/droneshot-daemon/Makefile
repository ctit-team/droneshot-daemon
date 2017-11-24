.PHONY: all clean emulation rpi

# common variables

OUTPUT = droneshot-daemon
CPPFLAGS = -I../droneshot-api/include -I../droneshot-lib/include
LDFLAGS = -L../droneshot-api/lib -L../droneshot-lib/lib
LDLIBS = -lstdc++ -ldroneshot-api -ldroneshot-lib

# sources

SOURCES = droneshot-daemon.c \
          rpc/rpc_server.c

# phony rules

ifeq ($(PLATFORM),emulation)
all: emulation
else
all: rpi
endif

clean:
	rm -f $(OUTPUT)
	find . -name '*.o' -delete

emulation: CPPFLAGS += -DDS_HW_EMULATION
emulation: $(OUTPUT)

rpi: LDLIBS += -lbcm2835
rpi: $(OUTPUT)

# file rules

ifeq ($(PLATFORM),emulation)
$(OUTPUT): $(SOURCES:.c=.o) \
           hardware/hardware_interface.o \
           hardware/emulation/hardware_interface.o
else
$(OUTPUT): $(SOURCES:.c=.o) \
           hardware/hardware_interface.o \
           hardware/rpi/hardware_interface.o
endif
