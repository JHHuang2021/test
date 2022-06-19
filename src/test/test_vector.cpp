#include <iostream>
#include <vector>
#include "../lib/vector.h"

int main() {
  // lin::vector<int> v;
  std::vector<int> v;
  v.push_back(3);
  v.push_back(2);
  v.push_back(1);

  auto it1 = v.begin(), it2 = v.end();
  it1 < it2;
  for (auto i : v) {
    std::cout << i << " ";
  }
  lin::vector<int> v2 = std::move(v);
  for (auto i : v2) {
    std::cout << i << " ";
  }
  return 0;
}