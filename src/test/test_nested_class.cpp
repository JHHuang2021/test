#include <iostream>
class AA {
 public:
  void funA() { std::cout << "funA" << std::endl; }
};

class A {
  AA a;

 public:
  class B {
   public:
    void funB() { a.funA(); }
  };
};
int main() {
  A::B b;
  return 0;
}