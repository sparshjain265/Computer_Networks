CC=gcc
CFLAGS=-Wall
SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

.PHONY: all
.PHONY: clean

protocol: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) -lrt -lpthread

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES)

clean:
	rm -rf protocol $(OBJECTS)
