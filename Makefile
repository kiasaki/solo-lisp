CC = gcc
CFLAGS = -std=c99 -Wall -g -Ilib
LIBS = -lm -ledit
TARGET = solo

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard lib/*.c))
OBJECTS += $(patsubst %.c, %.o, $(wildcard src/*.c))
HEADERS = $(wildcard lib/*.h)
HEADERS += $(wildcard src/*.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f lib/*.o
	-rm -f src/*.o
	-rm -f $(TARGET)
