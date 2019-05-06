CC = gcc
CFLAGS = -c -Wall -g
LDFLAGS = -Lsrc/third_party/iniparser -liniparser
SOURCES = main.c utils.c list.c logging.c
OBJECTS = $(SOURCES:.c=.o)

vpath %.c ./src/

EXECUTABLE = tmpfsp

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm $(OBJECTS)

.PHONY: clean-all
clean-all:
	rm -rf *.o && rm $(EXECUTABLE)

.PHONY: install
install:
	cp $(EXECUTABLE) /sbin/$(EXECUTABLE)
	cp configs/main.ini /etc/$(EXECUTABLE).ini
	chmod 511 /sbin/$(EXECUTABLE)
