#include <iostream>

#include "command_hash.h"
int main() {
  auto command = CommandHash::LookupCommand("logout", 6);
  if (command) {
    std::cout << "command name: " << command->name << ", code: " << command->code << std::endl;
  }
  return 0;
}