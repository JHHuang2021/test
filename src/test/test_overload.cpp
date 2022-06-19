#include <algorithm>
#include <vector>

class A {
 public:
  int val;
  friend bool cmp(const A &, const A &);
};
bool cmp(const A &a, const A &b) { return a.val < b.val; }
class B {
 public:
  int val;
  friend bool cmp(const B &, const B &);
};
bool cmp(const B &a, const B &b) { return a.val < b.val; }
int main() {
  std::vector<A> v;
  std::sort(v.begin(), v.end(), static_cast<bool (*)(const A&, const A&)>(cmp));
  return 0;
}