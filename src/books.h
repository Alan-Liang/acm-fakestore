#ifndef PANIC_BOOKSTORE_BOOKS_H_
#define PANIC_BOOKSTORE_BOOKS_H_

#include <ak/file/varchar.h>
#include <optional>
#include <string>

#include "bptree.h"

class Book {
 public:
  ak::file::Varchar<20> isbn;
  ak::file::Varchar<60> name, author, keyword;
  long long quantity = 0;
  long long price = 0;

  static void validateIsbn (const std::string &isbn);
  static void validateName (const std::string &name);
  static void validateAuthor (const std::string &author);
  static void validateKeyword (const std::string &keyword);
  static void validatePrice (long long price);

  static std::string Book::formatDecimal (long long decimal);

  Book() = default;
  bool operator< (const Book &rhs) const;
  // 修改与进货等直接访问成员变量。

  void print () const;
};

class BookManager {
  enum Field { kIsbn, kKeyword, kAuthor, kName, kPrice };
 private:
  BpTree<ak::file::Varchar<20>, Book> books_;
  BpTree<ak::file::Varchar<60>, ak::file::Varchar<20>> nameBooks_;
  BpTree<ak::file::Varchar<60>, ak::file::Varchar<20>> keywordBooks_;
  BpTree<ak::file::Varchar<60>, ak::file::Varchar<20>> authorBooks_;

  std::optional<Book> bookFromIsbn_ (const std::string &isbn);
 public:
  BookManager () = delete;
  BookManager (const char *bookfile, const char *keywordfile, const char *authorfile, const char *namefile);
  void show (Field field, const std::string &value);
  void show ();
  void buy (const std::string &isbn, const long long &cnt);
  void select (const std::string &isbn);
  struct UpdateClause {
    Field field;
    std::string payload;
  };
  void modify (const std::vector<UpdateClause> &updates);
};

#endif
