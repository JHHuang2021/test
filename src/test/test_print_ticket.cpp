#include <cstring>
#include <iostream>

#include "../lib/utils.h"

void test(int n) {
  std::string ret;
  std::string train_id = "HappyTrain";
  std::string a = "aaaaa", b = "bbB", c = "ccccc", d = "asdasda";
  int price = 121321, seat = 41321;
  // ret.reserve(1234567890ULL);
  for (int i = 1; i <= n; ++i) {
    // ret += '\n' + train_id + ' ' + a + ' ' + b + " -> " + c + ' ' +  //
    //        d + ' ' + std::to_string(price) + ' ' + std::to_string(seat);
    // ret += lin::concatenate('\n', train_id, ' ', a, ' ', b, " -> ", c, ' ',  //
    //                    d, ' ', std::to_string(price), ' ', std::to_string(seat));
    // ret += '\n';
    // ret += train_id;
    // ret += ' ';
    // ret += a;
    // ret += ' ';
    // ret += b;
    // ret += " -> ";
    // ret += c;
    // ret += ' ';
    // ret += d;
    // ret += ' ';
    // ret += std::to_string(price);
    // ret += ' ';
    // ret += std::to_string(seat);
    lin::append(ret, '\n', train_id, ' ', a, ' ', b, " -> ", c, ' ',  //
        d, ' ', std::to_string(price), ' ', std::to_string(seat));
  }
  // std::cout << ret;
}

int main() {
  int T = 1e5;
  for (int i = 1; i <= T; ++i) {
    test(30);
  }
  return 0;
}