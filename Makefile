CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -O2
LIBS = -lm -lz 
LDFLAGS = `pkg-config --libs gtk+-3.0` 
SOURCES = $(wildcard *.c)
OUTDIR = ./build/
EXECUTE = simple_comics_viewer

.PHONY: all

all: $(EXECUTE)

$(EXECUTE): $(SOURCES)
	$(CC) -o $(OUTDIR)$(EXECUTE) $(SOURCES) $(CFLAGS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm $(OUTDIR)$(EXECUTE)

.PHONY: check
check:
	$(OUTDIR)$(EXECUTE)
