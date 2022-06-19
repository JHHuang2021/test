#include <iostream>
#include <random>

template <class T>
class Base {
 public:
  friend bool operator==(const T &a, const T &b) { return !(a < b) && !(b < a); }
};

class A : Base<A> {
  int val_;

 public:
  A(int val = 0) : val_(val) {};
  friend bool operator<(const A &a, const A &b) { return a.val_ < b.val_; }
};

int main() {
  int n = 1e8, sum = 0;
  for (int i = 1; i <= n; ++i) {
    int ia = rand(), ib = rand();
    // sum += ia == ib;
    A a(ia), b(ib);
    sum += a == b;
  }
  return 0;
}