CFLAGS = -O2 `pkg-config --cflags gtk+-3.0`
LIBFLAGS = -lm -lz 
LIBSDIR = `pkg-config --libs gtk+-3.0` 
SOURCES = $(wildcard *.c)

run: main.c
	gcc -o build/simple_comics_viewer $(SOURCES) $(CFLAGS) $(LIBSDIR) $(LIBFLAGS)

.PHONY: clean
clean:
	rm build/simple_comics_viewer

.PHONY: check
check:
	./build/simple_comics_viewer
