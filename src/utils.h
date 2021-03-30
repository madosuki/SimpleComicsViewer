#ifndef COMICS_V_UTILS_H
#define COMICS_V_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
/* #include <zlib.h> */
#include <stdint.h>
#include <math.h>

#include <archive.h>

#include "loader.h"

#define UTILS_PNG 1
#define UTILS_JPG 2

#define UTILS_ZIP 1

extern const short png_sig_size;
extern const unsigned char png_sig[8];

extern const short jpg_sig_size;
extern const unsigned char jpg_sig[2];

/* extern const short zlib_sig_size; */
/* extern const unsigned char zlib_no_compression_or_low_sig[2]; */
/* extern const unsigned char zlib_default_compression_sig[2]; */
/* extern const unsigned char zlib_best_compression_sig[2]; */
extern const uint8_t compress_headers_flag;

double mygcd(double x, double y);

double *calc_aspect_raito(int width, int height, double gcd);

int detect_image(uint8_t *buf, int size);

int detect_image_from_file(const char *file_name);

char *get_directory_path_from_filename(const char *file_name);

int detect_compress_file(const char *file_name);

#endif // COMICS_V_UTILS_H
