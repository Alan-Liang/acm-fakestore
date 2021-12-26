#include "books.h"

#include <math.h>

#include <ak/compare.h>
#include <ak/validator.h>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

bool Book::operator< (const Book &rhs) const {
  return isbn < rhs.isbn;
}

namespace {
using ak::validator::expect;
} // namespace

void Book::validateIsbn (const std::string &isbn) {
  expect(isbn).toMatch(R"([\x21-\x7E]+)").butNot().toBeLongerThan(20);
}
void Book::validateName (const std::string &name) {
  expect(name).toMatch(R"([\x21-\x7E]+)").butNot().toBeLongerThan(60).toInclude("\"");
}
void Book::validateAuthor (const std::string &author) {
  expect(author).toMatch(R"([\x21-\x7E]+)").butNot().toBeLongerThan(60).toInclude("\"");
}
void Book::validateKeyword (const std::string &keyword) {
  expect(keyword).toMatch(R"([\x21-\x7E]+)").butNot().toBeLongerThan(60).toInclude("\"");
  std::set<std::string> keywords;
  std::istringstream iss(keyword);
  while (!iss.eof()) {
    std::string str;
    std::getline(iss, str, '|');
    if (keywords.contains(str)) throw std::exception();
    keywords.insert(str);
  }
}
void Book::validatePrice (long long price) {
  expect(price).toBeLessThan(10000000000000000LL);
}

std::string Book::formatDecimal (long long decimal) {
  std::string frac = std::to_string(decimal % 100);
  if (frac.length() == 1) frac = "0" + frac;
  return std::to_string(decimal / 100) + "." + frac;
}
long long Book::parseDecimal (const std::string &str) {
  expect(str).toBeConsistedOf("1234567890.").butNot().toBeLongerThan(13).toMatch(R"(\..*\.)");
  expect(str.substr(0, str.length() - 3)).toBeConsistedOf("1234567890");
  double d = std::stod(str);
  return round(d * 100);
}

void Book::print () const {
  std::cout
    << isbn.str() << '\t'
    << name.str() << '\t'
    << author.str() << '\t'
    << keyword.str() << '\t'
    << formatDecimal(price) << '\t'
    << quantity << '\n';
}

std::vector<std::string> Book::keywords() const {
  std::vector<std::string> keywords;
  std::istringstream iss(keyword);
  while (!iss.eof()) {
    std::string str;
    std::getline(iss, str, '|');
    if (!str.empty()) keywords.push_back(str);
  }
  return keywords;
}

BookManager::BookManager (
  const char *bookfile,
  const char *keywordfile,
  const char *authorfile,
  const char *namefile
) :
  books_(bookfile),
  keywordBooks_(keywordfile),
  authorBooks_(authorfile),
  nameBooks_(namefile) {}
std::optional<Book> BookManager::bookFromIsbn_ (const std::string &isbn) {
  Book::validateIsbn(isbn);
  std::vector<Book> book;
  books_.query(isbn, book);
  if (book.empty()) return std::nullopt;
  return book.front();
}
void BookManager::show (Field field, const std::string &value) {
  if (value.length() == 0) throw std::exception();
  expect(field).toBeOneOf({ kIsbn, kKeyword, kAuthor, kName });
  if (field == kIsbn) {
    auto book = bookFromIsbn_(value);
    if (!book) {
      std::cout << '\n';
      return;
    }
    book->print();
    return;
  }
  BpTree<ak::file::Varchar<60>, ak::file::Varchar<20>> *db;
  if (field == kKeyword) {
    db = &keywordBooks_;
    Book::validateKeyword(value);
    expect(value).Not().toInclude("|");
  }
  if (field == kAuthor) {
    Book::validateAuthor(value);
    db = &authorBooks_;
  }
  if (field == kName) {
    Book::validateName(value);
    db = &nameBooks_;
  }
  std::vector<ak::file::Varchar<20>> ids;
  db->query(value, ids);
  if (ids.empty()) {
    std::cout << '\n';
    return;
  }
  for (const auto &isbn : ids) {
    bookFromIsbn_(isbn)->print();
  }
}
void BookManager::show () {
  std::vector<std::pair<decltype(Book().isbn), Book>> res;
  books_.queryAll(res);
  if (res.empty()) {
    std::cout << '\n';
    return;
  }
  for (const auto &[ _, book ] : res) book.print();
}
long long BookManager::buy (const std::string &isbn, long long cnt) {
  auto book = bookFromIsbn_(isbn);
  if (!book) throw std::exception();
  expect(cnt).Not().toBeGreaterThan(2'147'483'647LL).toBeGreaterThan(book->quantity);

  // critical area begin
  books_.del(book->isbn, *book);
  book->quantity -= cnt;
  books_.add(book->isbn, *book);
  // critical area end

  long long price = book->price * cnt;
  std::cout << Book::formatDecimal(price) << '\n';
  return price;
}
Book BookManager::select (const std::string &isbn) {
  auto book = bookFromIsbn_(isbn);
  if (book) return *book;
  Book::validateIsbn(isbn);
  Book b;
  b.isbn = isbn;
  books_.add(b.isbn, b);
  authorBooks_.add(b.author, b.isbn);
  nameBooks_.add(b.name, b.isbn);
  return b;
}

Book BookManager::modify (const std::string &isbn, const std::vector<FieldClause> &updates) {
  expect(updates.size()).toBeGreaterThan(0);
  auto obook = bookFromIsbn_(isbn);
  if (!obook) throw std::exception();
  Book book = *obook;
  Book copy = book;
  std::set<Field> fieldsUpdated;
  for (const auto &update : updates) {
    if (update.payload.empty()) throw std::exception();
    switch (update.field) {
      case kIsbn: {
        Book::validateIsbn(update.payload);
        std::vector<Book> existing;
        books_.query(update.payload, existing);
        if (!existing.empty()) throw std::exception();
        copy.isbn = update.payload;
        break;
      }
      case kKeyword: {
        Book::validateKeyword(update.payload);
        copy.keyword = update.payload;
        break;
      }
      case kAuthor: {
        Book::validateAuthor(update.payload);
        copy.author = update.payload;
        break;
      }
      case kName: {
        Book::validateName(update.payload);
        copy.name = update.payload;
        break;
      }
      case kPrice: {
        long long price = Book::parseDecimal(update.payload);
        Book::validatePrice(price);
        copy.price = price;
        break;
      }
      default: throw std::exception();
    }
    if (fieldsUpdated.contains(update.field)) throw std::exception();
    fieldsUpdated.insert(update.field);
  }

  if (fieldsUpdated.contains(kIsbn)) {
    fieldsUpdated.insert(kAuthor);
    fieldsUpdated.insert(kKeyword);
    fieldsUpdated.insert(kName);
  }
  for (const Field &field : fieldsUpdated) {
    if (field == kAuthor) {
      authorBooks_.del(book.author, book.isbn);
      authorBooks_.add(copy.author, copy.isbn);
    }
    if (field == kKeyword) {
      for (const auto &kw : book.keywords()) keywordBooks_.del(kw, book.isbn);
      for (const auto &kw : copy.keywords()) keywordBooks_.add(kw, copy.isbn);
    }
    if (field == kName) {
      nameBooks_.del(book.name, book.isbn);
      nameBooks_.add(copy.name, copy.isbn);
    }
  }

  books_.del(book.isbn, book);
  book = copy;
  books_.add(book.isbn, book);
  return book;
}
void BookManager::import (const std::string &isbn, long long qty) {
  auto obook = bookFromIsbn_(isbn);
  if (!obook) throw std::exception();
  Book book = *obook;
  expect(qty).Not().toBeGreaterThan(2'147'483'647LL);
  books_.del(book.isbn, book);
  book.quantity += qty;
  books_.add(book.isbn, book);
}

void BookManager::clearCache () {
  books_.clearCache();
  authorBooks_.clearCache();
  keywordBooks_.clearCache();
  nameBooks_.clearCache();
}
