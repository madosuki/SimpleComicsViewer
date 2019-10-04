#include "utils.h"

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
