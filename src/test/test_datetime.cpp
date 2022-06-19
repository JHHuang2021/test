#include <iostream>

#include "../lib/datetime.h"

int main() {
  lin::Date dep_date("07-01");
  std::cout << dep_date.ToString() << std::endl;
  lin::Time dep_times[100], arr_times[100];
  dep_times[0] = lin::Time("19:43");
  int travel_times[] = {132, 91, 116, 36, 27, 68, 43, 179, 165, 142, 69, 201, 204};
  int stopover_times[] = {7, 6, 9, 9, 9, 4, 5, 4, 7, 8, 6, 3};
  int station_num = 14;
  for (int j = 0; j < station_num - 1; ++j) {
    arr_times[j + 1] = dep_times[j] + lin::Duration(travel_times[j]);
    if (j + 1 < station_num - 1) {
      dep_times[j + 1] = arr_times[j + 1] + lin::Duration(stopover_times[j]);
    }
  }
  std::cout << dep_times[3].minutes() << std::endl;
  // auto days = dep_times[3].GetDays();
  auto [days, time] = dep_times[3].GetDayTime();
  std::cout << days.minutes() << std::endl;
  lin::Date start_date = dep_date - days;
  std::cout << start_date.ToString() << std::endl;
  std::cout << (start_date + dep_times[3]).ToString() << std::endl;
  return 0;
}