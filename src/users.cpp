#include "users.h"

#include <ak/validator.h>
#include <string>
#include <vector>

User::User (
  const std::string &id,
  const std::string &name,
  const std::string &password,
  Privilege privilege
) :
  id_(id),
  name_(name),
  password_(password),
  privilege_(privilege) {}

void User::requestPrivilege_ (Privilege required) {
  if (privilege_ < required) throw std::exception();
}

bool User::operator< (const User &rhs) const {
  return id_ < rhs.id_;
}
void User::passwd (const std::string &newPassword) {
  password_ = newPassword;
}

std::string User::id () { return id_; }
std::string User::name () { return name_; }
std::string User::password () { return password_; }
Privilege User::privilege () { return privilege_; }

namespace {
using ak::validator::expect;
} // namespace

void User::validateId (const std::string &id) {
  expect(id).toMatch(R"([0-9a-zA-Z_]+)").butNot().toBeLongerThan(30);
}
void User::validatePassword (const std::string &password) {
  expect(password).toMatch(R"([0-9a-zA-Z_]+)").butNot().toBeLongerThan(30);
}
void User::validateName (const std::string &name) {
  expect(name).toMatch(R"([\x21-\x7E]+)").butNot().toBeLongerThan(30);
}
void User::validatePrivilege (Privilege privilege) {
  expect(privilege).toBeOneOf({ kCustomer, kWorker, kRoot });
}

User &UserManager::currentUser () {
  return userStack_.back().first;
}

std::optional<User> UserManager::userFromId_ (const std::string &id) {
  std::vector<User> res;
  users_.query(id, res);
  if (res.empty()) return std::nullopt;
  return res.front();
}

UserManager::UserManager (const char *filename) : users_(filename) {
  auto anon = userFromId_(kAnonymous);
  if (!anon) {
    anon = User(kAnonymous, kAnonymous, kAnonymous, kGuest);
    users_.add(anon->id(), *anon);
    users_.add(kAdminId, User(kAdminId, kAdminName, kAdminPassword, kRoot));
  }
  userStack_.emplace_back(*anon, "");
}

void UserManager::logIn (const std::string &id, const std::string &password) {
  if (id == kAnonymous) throw std::exception();
  auto user = userFromId_(id);
  if (!user) throw std::exception();
  if (!password.empty() && user->password() != password) throw std::exception();
  userStack_.emplace_back(*user, "");
}
void UserManager::logOut () {
  userStack_.pop_back();
}
void UserManager::signUp (const std::string &id, const std::string &password, const std::string &name) {
  User::validateId(id);
  User::validatePassword(password);
  User::validateName(name);
  std::vector<User> res;
  users_.query(id, res);
  if (!res.empty()) throw std::exception();
  User user(id, name, password, kCustomer);
  users_.add(user.id(), user);
}
void UserManager::userAdd (const std::string &id, const std::string &password, Privilege privilege, const std::string &name) {
  User::validateId(id);
  User::validatePassword(password);
  User::validateName(name);
  User::validatePrivilege(privilege);
  std::vector<User> res;
  users_.query(id, res);
  if (!res.empty()) throw std::exception();
  User user(id, name, password, privilege);
  users_.add(user.id(), user);
}
void UserManager::passwd (const std::string &id, const std::string &current, const std::string &newPassword) {
  auto user = userFromId_(id);
  if (!user || id == kAnonymous) throw std::exception();
  if (newPassword.empty()) {
    User::validatePassword(current);

    // critical area begin
    users_.del(user->id(), *user);
    user->passwd(current);
    users_.add(user->id(), *user);
    // critical area end

    return;
  }
  if (user->password() != current) throw std::exception();

  // critical area begin
  users_.del(user->id(), *user);
  user->passwd(newPassword);
  users_.add(user->id(), *user);
  // critical area end
}
void UserManager::remove (const std::string &id) {
  auto user = userFromId_(id);
  if (!user || id == kAnonymous) throw std::exception();
  for (const auto &[ user1, _ ] : userStack_) if (user->id_ == user1.id_) throw std::exception();
  users_.del(id, *user);
}

void UserManager::requestPrivilege (Privilege privilege) {
  currentUser().requestPrivilege_(privilege);
}
void UserManager::clearCache () {
  users_.clearCache();
}

std::string &UserManager::selection () {
  return userStack_.back().second;
}
void UserManager::updateSeletions (const std::string &old, const std::string &current) {
  for (auto &[ _, book ] : userStack_) if (book == old) book = current;
}
