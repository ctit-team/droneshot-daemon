.PHONY: all clean emulation rpi

# common variables

OUTPUT = droneshot-daemon

# tools option

CPPFLAGS = -O3 -Wall -g
LDFLAGS =
LDLIBS = `pkg-config --libs-only-l libuv`

ifeq ($(TYPE), systemd)
CPPFLAGS += -DDRONESHOT_SYSTEMD_SERVICE
LDLIBS += `pkg-config --libs-only-l libsystemd`
endif

ifeq ($(PLATFORM), emulation)
CPPFLAGS += -DDRONESHOT_HW_EMULATION
else
LDLIBS += -lbcm2835
endif

# sources

SOURCES = argument.c \
          droneshot-daemon.c \
          logging.c \
          hardware/transmitter_collection.c \
          rpc/client.c \
          rpc/parser.c \
          rpc/server.c \
          setting/main.c \
          setting/parser.c

ifeq ($(PLATFORM), emulation)
SOURCES += hardware/emulation/interface.c
else
SOURCES += hardware/rpi/interface.c
endif

# phony rules

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT)
	find . -name '*.o' -delete

# file rules

$(OUTPUT): $(SOURCES:.c=.o)
