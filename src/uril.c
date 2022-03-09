#include "util.h"

int log_2(int n) {
  int res = -1;
  while (n) {
    n >>= 1;
    res++;
  }
  return res;
}
