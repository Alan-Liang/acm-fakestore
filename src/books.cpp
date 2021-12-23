#include "books.h"

#include <ak/compare.h>
#include <ak/validator.h>
#include <iostream>
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
}
void Book::validatePrice (long long price) {
  expect(price).toBeLessThan(100000000000000LL);
}

std::string Book::formatDecimal (long long decimal) {
  std::string frac = std::to_string(decimal % 100);
  if (frac.length() == 1) frac = "0" + frac;
  return std::to_string(decimal / 100) + "." + frac;
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
  std::vector<Book> res;
  // TODO
}
void BookManager::buy (const std::string &isbn, const long long &cnt) {
  auto book = bookFromIsbn_(isbn);
  if (!book) throw std::exception();
  expect(cnt).Not().toBeGreaterThan(2'147'483'647LL).toBeGreaterThan(book->quantity);

  // critical area begin
  books_.del(book->isbn, *book);
  book->quantity -= cnt;
  books_.add(book->isbn, *book);
  // critical area end

  // TODO: store tx record
}
void BookManager::select (const std::string &isbn) {}
void BookManager::modify (const std::vector<UpdateClause> &updates) {}
