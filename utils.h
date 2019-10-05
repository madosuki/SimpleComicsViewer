#ifndef COMICS_V_UTILS_H
#define COMICS_V_UTILS_H

#include <stdlib.h>


const short png_sig_size;
const int png_sig[8];

const short jpg_sig_size;
const int jpg_sig[4];

const short zlib_sig_size; 
const int zlib_no_compression_or_low_sig[2];
const int zlib_default_compression_sig[2];
const int zlib_best_compression_sig[2];

int mygcd(int x, int y);

int *calc_aspect_raito(int width, int height, int gcd);

#endif // COMICS_V_UTILS_H
