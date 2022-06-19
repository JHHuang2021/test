#include <algorithm>
// #include <vector>
#include "../lib/utils.h"

bool cmp(const int &a, const int &b) { return a > b; }

#include <cstdio>
#include <iostream>
using namespace std;
const int maxn = 1e7 + 5;
int n, a[maxn];

// void Sort(int *begin, int *end) {
//   // printf("%lld %lld\n", begin, end);
//   if (begin >= end) return;
//   int *key = begin + rand() % (end - begin + 1);
//   std::swap(*begin, *key);
//   int pivot = *begin;
//   int *i = begin, *j = end;
//   while (i < j) {
//     while (i < j && *j >= pivot) --j;
//     *i = *j;
//     while (i < j && *i <= pivot) ++i;
//     *j = *i;
//   }
//   *i = pivot;
//   Sort(begin, i);
//   Sort(i + 1, end);
// };

void Sort(int l, int r) {
  // printf("%lld %lld\n", begin, end);
  if (l >= r) return;
  int key = l + rand() % (r - l + 1);
  std::swap(a[l], a[key]);
  int pivot = a[l];
  int i = l, j = r;
  while (i < j) {
    while (i < j && a[j] >= pivot) --j;
    if (i < j) a[i++] = a[j];
    while (i < j && a[i] <= pivot) ++i;
    if (i < j) a[j--] = a[i];
  }
  a[i] = pivot;
  Sort(l, i - 1);
  Sort(i + 1, r);
};

void quicksort(int low, int high) {
  if (low >= high) return;
  int i = low, j = high;
  int key = a[i + rand() % (j - i + 1)];
  while (i <= j) {
    while (a[j] > key) j--;
    while (a[i] < key) i++;
    if (i <= j) swap(a[i++], a[j--]);
  }
  quicksort(low, j);
  quicksort(i, high);
}
void QuickSort(int *low, int *high) {
  if (low >= high) return;
  int *i = low, *j = high;
  int key = *(i + rand() % (j - i + 1));
  while (i <= j) {
    while (i <= j && *j > key) j--;
    while (i <= j && *i < key) i++;
    if (i <= j) swap(*(i++), *(j--));
  }
  QuickSort(low, j);
  QuickSort(i, high);
}

int main() {
  std::sort(a, a+2, cmp);
  n = 1e5;
  for (int i = 1; i <= n; ++i) a[i] = 1;
  // Sort(1, n);
  quicksort(1, n);
  return 0;
}

// int main() {
//   int v[] = {3,6,1,2,8};
//   lin::Sort(v, v + 5, cmp);
//   for(int i=0;i<5;++i) printf("%d ", v[i]);
// }