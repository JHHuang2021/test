#include <iostream>
#include <random>

#include "../lib/tuple.h"

int main() {
  std::cout << (lin::Tuple(1, 2) < lin::Tuple(1, 2)) << std::endl;
  std::cout << (lin::Tuple(1, 2) <= lin::Tuple(1, 2)) << std::endl;
  int n = 1e7, sum = 0;
  for (int i = 1; i <= n; ++i) {
    // sum += (i < i + 1) + (i + 1 < i + 2);
    int a = rand(), b = rand(), c = rand(), d = rand();
    int ans1 = a == b ? c <= d : a < b;
    int ans2 = lin::Tuple(a, c) <= lin::Tuple(b, d);
    if (ans1 != ans2) {
      printf("%d,%d %d,%d\n", a, c, b, d);
    }
    sum += ans1;
  }
  std::cout << sum << std::endl;
  return 0;
}