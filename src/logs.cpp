#include "logs.h"

#include <ak/chalk.h>
#include <sstream>
#include <iostream>

#include "books.h"

TradeRecord::TradeRecord (const bool &isExpense, long long amount) : isExpense_(isExpense) {
  (isExpense ? expense_ : income_) = amount;
}
TradeRecord &TradeRecord::operator+= (const TradeRecord &rhs) {
  expense_ += rhs.expense_;
  income_ += rhs.income_;
  return *this;
}
std::ostream &operator<< (std::ostream &os, const TradeRecord &rec) {
  os << "+ " << Book::formatDecimal(rec.income_) << " - " << Book::formatDecimal(rec.expense_) << "\n";
  return os;
}
void TradeRecord::prettyPrint () {
  std::string tag = isExpense_ ? ak::chalk::red("expense") : ak::chalk::green("income");
  std::string amount = Book::formatDecimal(isExpense_ ? expense_ : income_);
  std::cout << tag << " " << amount << std::endl;
}

CmdRecord::CmdRecord (const std::string &userId, const std::string &command) : userId_(userId), command_(command) {}
std::ostream &operator<< (std::ostream &os, const CmdRecord &rec) {
  std::istringstream iss(rec.command_);
  std::string argv0, args;
  std::getline(iss, argv0, ' ');
  std::getline(iss, args, '\n');
  os << "[" << ak::chalk::magenta(rec.userId_.str()) << "] " << ak::chalk::bold(ak::chalk::blue(argv0)) << " " << args << '\n';
  return os;
}

void CmdRecord::printIfIsUser (const std::string &userId) {
  if (userId == userId_.str()) std::cout << *this;
}

int LogManager::tradeCount_ () {
  int i;
  tradeFile_.get(&i, 0, sizeof(i));
  return i;
}
int LogManager::cmdCount_ () {
  int i;
  cmdFile_.get(&i, 0, sizeof(i));
  return i;
}
LogManager::LogManager (const std::string &name) : tradeFile_((name + "_trade.bin").c_str(), [this] {
  int i = 0;
  tradeFile_.push(&i, sizeof(i));
}), cmdFile_((name + "_cmd.bin").c_str(), [this] {
  int i = 0;
  cmdFile_.push(&i, sizeof(i));
}) {}
void LogManager::addTrade (const TradeRecord &rec) {
  tradeFile_.push(&rec, sizeof(rec));
  int id = tradeCount_() + 1;
  tradeFile_.set(&id, 0, sizeof(id));
}
void LogManager::showFinance (int cnt) {
  if (cnt == 0) {
    std::cout << '\n';
    return;
  }
  int count = tradeCount_();
  if (cnt > count) throw std::exception();
  TradeRecord rec(false, 0);
  for (int i = count - cnt + 1; i <= count; ++i) {
    TradeRecord current(false, 0);
    tradeFile_.get(&current, i, sizeof(current));
    rec += current;
  }
  std::cout << rec;
}
void LogManager::showFinance () {
  showFinance(tradeCount_());
}
void LogManager::reportFinance () {
  int sz = tradeCount_();
  for (int i = 1; i <= sz; ++i) {
    TradeRecord rec;
    tradeFile_.get(&rec, i, sizeof(rec));
    std::cout << i << ". ";
    rec.prettyPrint();
  }
  std::cout << ak::chalk::magenta(ak::chalk::bold("Total")) << ": ";
  showFinance();
}
void LogManager::addLog (const CmdRecord &rec) {
  cmdFile_.push(&rec, sizeof(rec));
  int id = cmdCount_() + 1;
  cmdFile_.set(&id, 0, sizeof(id));
}
void LogManager::reportEmployee (const std::string &id_) {
  std::cout << "Actions performed by " << ak::chalk::magenta(ak::chalk::bold(id_)) << ":" << std::endl;
  int sz = cmdCount_();
  for (int i = 1; i <= sz; ++i) {
    CmdRecord rec;
    cmdFile_.get(&rec, i, sizeof(rec));
    rec.printIfIsUser(id_);
  }
}
void LogManager::reportLog () {
  const char dashes[] = "--------------------";
  std::cout << dashes << ak::chalk::red(ak::chalk::bold(" Finance Report ")) << dashes << std::endl;
  reportFinance();
  std::cout << std::endl;
  std::cout << dashes << ak::chalk::red(ak::chalk::bold(" System Logs ")) << dashes << std::endl;
  int sz = cmdCount_();
  for (int i = 1; i <= sz; ++i) {
    CmdRecord rec;
    cmdFile_.get(&rec, i, sizeof(rec));
    std::cout << rec;
  }
}

void LogManager::clearCache () {
  cmdFile_.clearCache();
  tradeFile_.clearCache();
}
