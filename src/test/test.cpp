#include <iostream>
#include <vector>

#include "lib/datetime.h"

template <class... Ts>
void print(Ts const&... args) {
  ((std::cout << args << " "), ...);
  std::cout << "\n";
}

int main() {
  lin::Duration dt(31);
  lin::DateTime t1("07-31 23:59"), t2("08-01 8:03");
  auto dlt = t2 - t1;
  print(dlt.minutes());
  // for (int i = 1; i <= 100000000; ++i) {
  //   auto date = time.GetTime();
  //   // printf("%d %d", date.first, date.second);
  // }
  auto date = t1.GetDate();
  printf("%d %d\n", date.first, date.second);
  auto t = t1.GetTime();
  printf("%d %d\n", t.first, t.second);
  std::cout << t1.ToString() << std::endl;
  std::cout << (t1 + dt).ToString() << std::endl;

  std::vector<int> v;
  return 0;
}