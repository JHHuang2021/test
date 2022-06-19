#include <iostream>
class A {
 public:
  int val;
  A(int value) : val(value) {}
  friend A operator+(const A &a, const A &b);
};
A operator+(const A &a, const A &b) { return A(a.val + b.val); }
class B {
 public:
  int val;
  B(int value) : val(value) {}
  operator A() { return A(val); }
};
int main() {
  A a{1};
  B b1{1}, b2{2};
  a + b1;
  b1 + b2;
  return 0;
}