#include "utils.h"

const short png_sig_size = 8;
const char png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};

const short jpg_sig_size = 4;
const char jpg_sig[4] = {255, 216, 255, 219};

const short zlib_sig_size = 2; 
const char zlib_no_compression_or_low_sig[2] = {120, 1};
const char zlib_default_compression_sig[2] = {120, 156};
const char zlib_best_compression_sig[2] = {120, 218};

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

int *calc_aspect_raito(int width, int height, int gcd)
{
  int width_aspect = width / gcd;
  int height_aspect = height / gcd;

  int *tuple = (int*)calloc(2, sizeof(int));

  tuple[0] = width_aspect;
  tuple[1] = height_aspect;
  
  return tuple;
}
