#include "logs.h"

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

CmdRecord::CmdRecord (const std::string &userId, const std::string &command) : userId_(userId), command_(command) {}
std::ostream &operator<< (std::ostream &os, const CmdRecord &rec) {
  os << "[" << rec.userId_.str() << "] " << rec.command_.str() << '\n';
  return os;
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
  // TODO
  std::cout << "Method not implemented\n";
}
void LogManager::addLog (const CmdRecord &rec) {
  cmdFile_.push(&rec, sizeof(rec));
  int id = cmdCount_() + 1;
  cmdFile_.set(&id, 0, sizeof(id));
}
void LogManager::reportEmployee (const std::string &id_) {
  // TODO
  std::cout << "Method not implemented\n";
}
void LogManager::reportLog () {
  // TODO
  std::cout << "Method not implemented\n";
}

void LogManager::clearCache () {
  cmdFile_.clearCache();
  tradeFile_.clearCache();
}
