#include <iostream>
#include <tuple>

#include "../lib/tuple.h"

int main() {
  // int a = 1, b = 2;
  // auto c = a <=> b;
  // int a, b, c, d;
  // std::cout << (std::tie(a, b, c, d) < std::tie(a, b, c, d));
  lin::Tuple<int, int> t;
  lin::Tuple<int, int> t1(7, 8), t2(8, 8);
  // t.get<0>() = 1;
  // t.get<1>() = 2;
  auto r1 = t1 < t2, r2 = t2 < t1;
  std::cout << r1 << " " << r2 << std::endl;
  return 0;
}