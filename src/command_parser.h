#pragma once

#include <iostream>  // for std::string

#include "lib/vector.h"

namespace lin {
class UserManager;
class TrainManager;
/**
 * @brief 解析输入指令，并调用相应的函数。
 */
class CommandParser {
 public:
  CommandParser(UserManager *user_manager, TrainManager *train_manager)
      : user_manager_(user_manager), train_manager_(train_manager) {}
  /**
   * @brief 循环读入指令并解析，直到遇到 exit。
   */
  void Run();

 private:
  UserManager *user_manager_;
  TrainManager *train_manager_;
  int timestamp;
  vector<char *> argv;
  /**
   * @brief Splits a C-style string by blank character.
   * Note that the original string WILL BE MODIFIED.
   */
  template <typename T = char *>
  static vector<T> Split(char *s, const char sep = ' ');
  /**
   * @brief Splits a C-style string by blank character, stores the result in the given array.
   * Note that the original string WILL BE MODIFIED.
   */
  template <typename T = char *>
  static void Split(char *s, T result[], const char sep = ' ');
  /**
   * @brief Gets a positive number from a C-style string WITHOUT checking the character is a digit.
   */
  static int ParseNumber(const char *s);
  std::string ParseAddUser();
  std::string ParseLogin();
  std::string ParseLogout();
  std::string ParseQueryProfile();
  std::string ParseModifyProfile();
  std::string ParseAddTrain();
  std::string ParseDeleteTrain();
  std::string ParseReleaseTrain();
  std::string ParseQueryTrain();
  std::string ParseQueryTicket();
  std::string ParseQueryTransfer();
  std::string ParseBuyTicket();
  std::string ParseQueryOrder();
  std::string ParseRefundTicket();
  std::string ParseRollback();
  std::string ParseClean();
};
}  // namespace lin