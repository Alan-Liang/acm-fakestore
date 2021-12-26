#include <ak/validator.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "books.h"
#include "users.h"
#include "logs.h"

BookManager::FieldClause parseClause (const std::string &arg) {
  if (arg.length() < 2) throw std::exception();
  if (arg[1] == 'I') {
    ak::validator::expect(arg).toMatch(R"(-ISBN=.+)");
    return { .field = BookManager::Field::kIsbn, .payload = arg.substr(6) };
  }
  if (arg[1] == 'n') {
    ak::validator::expect(arg).toMatch(R"(-name=".+")");
    return { .field = BookManager::Field::kName, .payload = arg.substr(7, arg.length() - 8) };
  }
  if (arg[1] == 'a') {
    ak::validator::expect(arg).toMatch(R"(-author=".+")");
    return { .field = BookManager::Field::kAuthor, .payload = arg.substr(9, arg.length() - 10) };
  }
  if (arg[1] == 'k') {
    ak::validator::expect(arg).toMatch(R"(-keyword=".+")");
    return { .field = BookManager::Field::kKeyword, .payload = arg.substr(10, arg.length() - 11) };
  }
  if (arg[1] == 'p') {
    ak::validator::expect(arg).toMatch(R"(-price=.+)");
    return { .field = BookManager::Field::kPrice, .payload = arg.substr(7) };
  }
  throw std::exception();
};

int main () {
  BookManager bookManager("books.dat", "keyword_books.dat", "author_books.dat", "name_books.dat");
  UserManager userManager("users.dat");
  LogManager logManager("log");

  while (!std::cin.eof()) {
    std::string rawCommand;
    std::getline(std::cin, rawCommand);
    if (rawCommand.size() > 1024) {
      std::cout << "Invalid\n";
      continue;
    }
    if (rawCommand.empty()) continue;
    std::vector<std::string> args;
    std::istringstream iss(rawCommand);
    while (!iss.eof()) {
      std::string arg;
      std::getline(iss, arg, ' ');
      if (!arg.empty()) args.push_back(arg);
    }
    if (args.empty()) continue;
    auto nary = [&args] (int i) { if (args.size() != i + 1) throw std::exception(); };
    try {
      if (args[0] == "quit" || args[0] == "exit") {
        nary(0);
        return 0;
      }
      if (args[0] == "su") {
        if (args.size() == 2) {
          userManager.logIn(args[1]);
        } else if (args.size() == 3) {
          userManager.logIn(args[1], args[2]);
        } else {
          throw std::exception();
        }
      } else if (args[0] == "logout") {
        nary(0);
        userManager.requestPrivilege(kCustomer);
        userManager.logOut();
      } else if (args[0] == "register") {
        nary(3);
        userManager.signUp(args[1], args[2], args[3]);
      } else if (args[0] == "passwd") {
        userManager.requestPrivilege(kCustomer);
        if (args.size() == 3) {
          userManager.requestPrivilege(kRoot);
          userManager.passwd(args[1], args[2]);
        } else if (args.size() == 4) {
          userManager.passwd(args[1], args[2], args[3]);
        } else {
          throw std::exception();
        }
      } else if (args[0] == "useradd") {
        nary(4);
        userManager.requestPrivilege(kWorker);
        Privilege p;
        if (args[3] == "1") {
          p = kCustomer;
          userManager.requestPrivilege(kWorker);
        }
        else if (args[3] == "3") {
          p = kWorker;
          userManager.requestPrivilege(kRoot);
        }
        else throw std::exception();
        userManager.userAdd(args[1], args[2], p, args[4]);
      } else if (args[0] == "delete") {
        nary(1);
        userManager.requestPrivilege(kRoot);
        userManager.remove(args[1]);
      } else if (args[0] == "show") {
        if (args.size() > 1 && args[1] == "finance") {
          userManager.requestPrivilege(kRoot);
          if (args.size() == 2) {
            logManager.showFinance();
          } else if (args.size() == 3) {
            ak::validator::expect(args[2]).toBeConsistedOf("1234567890").butNot().toBeLongerThan(10);
            long long time = std::stoll(args[2]);
            ak::validator::expect(time).Not().toBeGreaterThan(2'147'483'647LL);
            logManager.showFinance(time);
          } else {
            throw std::exception();
          }
        } else {
          userManager.requestPrivilege(kCustomer);
          if (args.size() == 1) {
            bookManager.show();
          } else if (args.size() == 2) {
            BookManager::FieldClause clause = parseClause(args[1]);
            if (clause.field == BookManager::Field::kPrice) throw std::exception();
            bookManager.show(clause.field, clause.payload);
          } else {
            throw std::exception();
          }
        }
      } else if (args[0] == "buy") {
        nary(2);
        userManager.requestPrivilege(kCustomer);
        ak::validator::expect(args[2]).toBeConsistedOf("1234567890").butNot().toBeLongerThan(10);
        long long qty = std::stoll(args[2]);
        long long price = bookManager.buy(args[1], qty);
        logManager.addTrade(TradeRecord(false, price));
      } else if (args[0] == "select") {
        nary(1);
        userManager.requestPrivilege(kWorker);
        Book book = bookManager.select(args[1]);
        userManager.selection() = book.isbn;
      } else if (args[0] == "modify") {
        ak::validator::expect(args.size()).toBeGreaterThan(1);
        userManager.requestPrivilege(kWorker);
        std::string isbn = userManager.selection();
        if (isbn.empty()) throw std::exception();
        std::vector<BookManager::FieldClause> updates;
        bool updateIsbn = false;
        for (int i = 1; i < args.size(); ++i) {
          auto update = parseClause(args[i]);
          if (update.field == BookManager::Field::kIsbn) updateIsbn = true;
          updates.push_back(update);
        }
        Book book = bookManager.modify(isbn, updates);
        if (updateIsbn) userManager.updateSeletions(isbn, book.isbn);
      } else if (args[0] == "import") {
        nary(2);
        userManager.requestPrivilege(kWorker);
        std::string isbn = userManager.selection();
        if (isbn.empty()) throw std::exception();
        ak::validator::expect(args[1]).toBeConsistedOf("1234567890").butNot().toBeLongerThan(10);
        long long qty = std::stoll(args[1]);
        long long totalCost = Book::parseDecimal(args[2]);
        bookManager.import(isbn, qty);
        logManager.addTrade(TradeRecord(true, totalCost));
      } else if (args[0] == "report") {
        // TODO
        throw std::exception();
      } else if (args[0] == "log") {
        // TODO
        throw std::exception();
      } else {
        throw std::exception();
      }
      logManager.addLog(CmdRecord(userManager.currentUser().id(), rawCommand));
    } catch (...) {
      std::cout << "Invalid\n";
    }
    bookManager.clearCache();
    userManager.clearCache();
    logManager.clearCache();
  }
}
