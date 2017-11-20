.PHONY: all clean

OUTPUT = droneshot-daemon
CXXFLAGS =
SOURCE = droneshot-daemon.cpp

all: $(OUTPUT)

clean:
	rm $(OUTPUT) *.o

$(OUTPUT): $(SOURCE:.cpp=.o)
