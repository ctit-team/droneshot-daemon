.PHONY: all clean emulation rpi

# common variables

OUTPUT = droneshot-daemon
CPPFLAGS = -O3 -Wall -g
LDFLAGS =
LDLIBS = `pkg-config --libs-only-l libuv`

ifeq ($(TYPE), systemd)
CPPFLAGS += -DDRONESHOT_SYSTEMD_SERVICE
LDLIBS += `pkg-config --libs-only-l libsystemd`
endif

# sources

SOURCES = droneshot-daemon.c \
          hardware/interface.c \
          hardware/transmitter_collection.c \
          rpc/client.c \
          rpc/parser.c \
          rpc/server.c

# phony rules

ifeq ($(PLATFORM),emulation)
all: emulation
else
all: rpi
endif

clean:
	rm -f $(OUTPUT)
	find . -name '*.o' -delete

emulation: CPPFLAGS += -DDRONESHOT_HW_EMULATION
emulation: $(OUTPUT)

rpi: LDLIBS += -lbcm2835
rpi: $(OUTPUT)

# file rules

ifeq ($(PLATFORM),emulation)
$(OUTPUT): $(SOURCES:.c=.o) \
           hardware/emulation/interface.o
else
$(OUTPUT): $(SOURCES:.c=.o) \
           hardware/rpi/interface.o
endif
