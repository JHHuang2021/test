#include <iostream>
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(NULL);
  int n = 5e6;
  for (int i = 1; i <= n; ++i) {
    std::cout << '[' << i << "]\n";
    // printf("[%d]\n", i);
  }
  return 0;
}