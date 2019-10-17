#include "utils.h"

const short png_sig_size = 8;
const unsigned char png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};

const short jpg_sig_size = 4;
const unsigned char jpg_sig[4] = {255, 216, 255, 219};

const short zlib_sig_size = 2; 
const unsigned char zlib_no_compression_or_low_sig[2] = {120, 1};
const unsigned char zlib_default_compression_sig[2] = {120, 156};
const unsigned char zlib_best_compression_sig[2] = {120, 218};

int mygcd(int x, int y)
{
  int tmp = x % y;

  while(1)
  {
      if(y % tmp == 0)
      {
          return tmp;
      }

      tmp = y % tmp;
  }
}

double *calc_aspect_raito(int width, int height, int gcd)
{
  int width_aspect = width / gcd;
  int height_aspect = height / gcd;

  double *tuple = (double*)calloc(2, sizeof(double));

  tuple[0] = (double)width_aspect;
  tuple[1] = (double)height_aspect;
  
  return tuple;
}

int detect_image(uint8_t *buf, int size)
{
    if(size < jpg_sig_size) {
        return 0;
    }

    int count = 0;
    if(buf[0] == png_sig[0]) {
        count++;

        for(int i = 1; i < png_sig_size; ++i) {
            if(buf[i] == png_sig[i]) {
                count++;
            }
        }

        if(count == png_sig_size) {
            return UTILS_PNG;
        }

    } else if(buf[0] == jpg_sig[0]) {
        count++;

        for(int i = 1; i < jpg_sig_size; ++i) {
            if(buf[i] == jpg_sig[i]) {
                count++;
            }
        }

        if(count == jpg_sig_size) {
            return UTILS_JPG;
        }
    }

    return 0;
}
