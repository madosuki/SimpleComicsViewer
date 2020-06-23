bin_PROGRAMS = simple_comics_viewer

simple_comics_viewer_SOURCES = main.c loader.c loader.h utils.c utils.h viewer.c viewer.h pdf_loader.c pdf_loader.h

simple_comics_viewer_CFLAGS = -O2 @GTK_CFLAGS@  

simple_comics_viewer_LDADD = -lmupdf -lmupdf-third -lz -lm -lpthread -lfreetype -lharfbuzz -ljpeg -lpng -lopenjp2 -ljbig2dec @GTK_LIBS@

CLEANFILES = *~
