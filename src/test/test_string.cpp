#include <iostream>
// #include <format>
// #define FMT_HEADER_ONLY
// #include <fmt/core.h>

#include "../lib/datetime.h"

inline void SetNumber(char *s, int num) {
  s[0] = (char(num / 10) + '0');
  s[1] = (char(num % 10) + '0');
}

inline void SetNumber(std::string &s, int num) {
  s += (char(num / 10) + '0');
  s += (char(num % 10) + '0');
}

void WriteTo(lin::DateTime time, std::string &s) {
  auto [month, day] = time.GetMonthDay();
  auto [hour, min] = time.GetHourMinute();
  // char buf[12];
  // std::string buf(12, '\0');
  // s.reserve(s.size() + 12);
  // str:  MM-dd hh:mm
  // pos:  0123456789
  SetNumber(s, month), s += ('-'), SetNumber(s, day);
  s.push_back(' ');
  SetNumber(s, hour), s += (':'), SetNumber(s, min);
}

void cap() {
  std::string s;
  lin::DateTime time(123214);
  int n = 1e7;
  for (int i = 1; i <= n; ++i) {
    s += time.ToString();
    // std::cout << s.capacity() << std::endl;

    // auto [month, day] = time.GetMonthDay();
    // auto [hour, min] = time.GetHourMinute();
    // int siz = s.size();
    // s.resize(siz + 12, '\0');
    // char *buf = s.data() + siz;
    // SetNumber(buf, month), buf[2] = '-', SetNumber(buf + 3, day);
    // buf[5] = ' ';
    // SetNumber(buf + 6, hour), buf[8] = ':', SetNumber(buf + 9, min);
  }
  // std::cout << s << std::endl;
}

int main() {
  std::string s = "1";
  std::cout << s.size() << std::endl;
  s.resize(2);
  std::cout << s << s << std::endl;
  std::cout << s.size() << std::endl;
  std::cout << s.length() << std::endl;
  // s.reserve(123);
  // std::cout << s.capacity();
  // cap();
  // std::string s;
  // lin::DateTime time(123214);
  // int n = 1e7;
  // for (int i = 1; i <= n; ++i) {
  //   // std::cout << s.capacity() << std::endl;
  //   s += time.ToString();
  //   // auto [MM, dd] = time.GetMonthDay();
  //   // auto [hh, mm] = time.GetHourMinute();
  //   // s += fmt::format("{}-{} {}:{}", MM, dd, hh, mm);
  //   // WriteTo(time, s);
  // }
  // std::cout << s << std::endl;
  // std::cout << s.capacity() << std::endl;
  return 0;
}