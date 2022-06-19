#include <iostream>
#include <map>
int main() {
  std::map<int, int> m;
  m[1] = 2;
  auto it = m.find(1);
  auto &x = it->second;
  x = 666;
  std::cout << m[1];
  return 0;
}