#include <iostream>

#include "../train.h"

lin::TrainSeats fun() { return lin::TrainSeats(1, 10); }

int main() {
  int n = 1e8, sum = 0;
  for (int i = 1; i <= n; ++i) {
    // lin::TrainSeats s = fun();
    lin::TrainSeats s(1, 10);
    sum += s.seat_num[1];
  }
  std::cout << sum << std::endl;
  return 0;
}