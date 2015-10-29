CC=gcc

CFLAGS=-Wall
LDFLAGS=-std=c99

SOURCES=main.c jfes.c examples/example_1.c examples/example_2.c examples/example_3.c

OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=jfes_demo

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -r $(EXECUTABLE)
