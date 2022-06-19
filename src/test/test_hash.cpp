#include <iostream>
// #include<unordered_map>
#include "../lib/char.h"
#include "../lib/hash.h"

int main() {
  std::string s;
  std::hash<std::string>()(s);
  // std::unordered_map<std::string, int> users;
  // users.emplace("sda", 1);
  Char<20> train_id("Hello");
  lin::Hasher<Char<20>> hasher;
  std::cout << hasher(train_id) << std::endl;
  std::cout << std::hash<std::string>()("Hello") << std::endl;
  return 0;
}