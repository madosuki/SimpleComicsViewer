#ifndef COMICS_V_UTILS_H
#define COMICS_V_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define UTILS_PNG 1
#define UTILS_JPG 2

const short png_sig_size;
const unsigned char png_sig[8];

const short jpg_sig_size;
const unsigned char jpg_sig[4];

const short zlib_sig_size; 
const unsigned char zlib_no_compression_or_low_sig[2];
const unsigned char zlib_default_compression_sig[2];
const unsigned char zlib_best_compression_sig[2];

int mygcd(int x, int y);

double *calc_aspect_raito(int width, int height, int gcd);

int detect_image(uint8_t *buf, int size);

#endif // COMICS_V_UTILS_H
