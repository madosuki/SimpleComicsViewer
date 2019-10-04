run: main.c
	gcc `pkg-config --cflags gtk+-3.0` -o simple_comics_viewer main.c mainwindow.c utils.c `pkg-config --libs gtk+-3.0` -lm

clean:
	rm app
