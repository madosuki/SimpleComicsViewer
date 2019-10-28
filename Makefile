run: main.c
	gcc `pkg-config --cflags gtk+-3.0` -o build/simple_comics_viewer main.c viewer.c utils.c loader.c `pkg-config --libs gtk+-3.0` -lm -lz -O2

clean:
	rm build/simple_comics_viewer
