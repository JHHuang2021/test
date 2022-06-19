#include "../lib/char.h"
int main() {
  std::string s = "aaa";
  Char<20> id;
  // const char *id = "1234";
  s += id.c_str();
  s.data();
  std::cout << s << std::endl;
}