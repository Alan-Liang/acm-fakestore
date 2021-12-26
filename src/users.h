#ifndef PANIC_BOOKSTORE_USERS_H_
#define PANIC_BOOKSTORE_USERS_H_

#include <ak/file/varchar.h>
#include <string>
#include <optional>

#include "bptree.h"
#include "books.h"

enum Privilege { kGuest = 0, kCustomer = 1, kWorker = 3, kRoot = 7 };

class User {
 private:
  ak::file::Varchar<30> id_, name_, password_;
  Privilege privilege_;

  void requestPrivilege_ (Privilege required);
  friend class UserManager;
 public:
  User () = default;
  User (const std::string &id, const std::string &name, const std::string &password, Privilege privilege);

  std::string id ();
  std::string name ();
  std::string password ();
  Privilege privilege ();

  static void validateId (const std::string &id);
  static void validatePassword (const std::string &password);
  static void validateName (const std::string &name);
  static void validatePrivilege (Privilege privilege);

  bool operator< (const User &rhs) const;
  void passwd (const std::string &newPassword);
};

class UserManager {
 private:
  // key 为 user id
  BpTree<ak::file::Varchar<30>, User> users_;
  std::vector<std::pair<User, std::string>> userStack_;

  std::optional<User> userFromId_ (const std::string &id);
  static constexpr const char *kAnonymous = "<anonymous>";
  static constexpr const char *kAdminId = "root";
  static constexpr const char *kAdminName = "";
  static constexpr const char *kAdminPassword = "sjtu";
 public:
  UserManager () = delete;
  // 这里应该初始化 userStack_ 为只有一个匿名帐号
  UserManager (const char *filename);
  User &currentUser ();
  void logIn (const std::string &id, const std::string &password = "");
  void logOut ();
  void signUp (const std::string &id, const std::string &password, const std::string &name);
  void userAdd (const std::string &id, const std::string &password, Privilege p, const std::string &name);
  void passwd (const std::string &id, const std::string &current, const std::string &newPassword = "");
  void remove (const std::string &id);

  void requestPrivilege (Privilege privilege);
  void clearCache ();

  std::string &selection ();
  void updateSeletions (const std::string &old, const std::string &current);
};

#endif
