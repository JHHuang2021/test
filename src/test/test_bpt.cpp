#include "../b_plus_tree/include/b_plus_tree.hpp"

int main() {
  huang::BPlusTree<int, int> bpt("test_bpt.dat", 1024);
  bpt.Insert(1, 1);
  auto [val, exist] = bpt.GetValue(1);
  printf("%d %d\n", *val, exist);
  return 0;
}