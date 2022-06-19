#include <iostream>
#include <string_view>
int tot;
// void fun(const std::string &s) { tot ^= s[0]; }
void fun(std::string_view s) { tot ^= s[0]; }
// void fun(const char *s) { tot ^= s[0]; }
int main() {
  // char buf[1024] = "Hello World!";
  // std::string_view s{buf};
  // std::cout << s << std::endl;
  // std::cout << s.length() << std::endl;
  // buf[13] = 'x';
  // char s[] = "Hello World!";
  std::string s = "Hello World!";
  long long n = 1e10;
  for (long long i = 1; i <= n; ++i) {
    fun(s);
  }
  std::cout << tot << std::endl;
  return 0;
}