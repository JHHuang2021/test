#include "train.h"
#include "user.h"
// #include "train.h"
// #include "order.h"
#include "command_parser.h"

int main() {
//   freopen("data/basic_2/1.in", "r", stdin);
//   freopen("test.out", "w", stdout);
  lin::UserManager user_manager;
  lin::TrainManager train_manager;
  lin::CommandParser command_parser(&user_manager, &train_manager);
  command_parser.Run();
  return 0;
}