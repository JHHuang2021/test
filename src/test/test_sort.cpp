#include <iostream>
#include <vector>

template <typename RandomIt, typename Compare = std::less<typename RandomIt::value_type>>
void QuickSort(RandomIt first, RandomIt last, Compare compare) {
  if (std::distance(first, last) > 1) {
    RandomIt bound = Partition(first, last, compare);
    QuickSort(first, bound, compare);
    QuickSort(bound + 1, last, compare);
  }
}

template <typename RandomIt, typename Compare>
RandomIt Partition(RandomIt first, RandomIt last, Compare compare) {
  auto pivot = std::prev(last, 1);
  auto i = first;
  for (auto j = first; j != pivot; ++j) {
    // bool format
    if (compare(*j, *pivot)) {
      std::swap(*i++, *j);
    }
  }
  std::swap(*i, *pivot);
  return i;
}
int main() {
  std::vector<int> v;

  return 0;
}