#include <iostream>

#include "../lib/datetime.h"

int main() {
  lin::Duration a{1}, b{2};
  std::cout << (a < b) << std::endl;
  std::cout << (a == b) << std::endl;
  std::cout << (a > b) << std::endl;
  return 0;
}