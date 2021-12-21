# BookStore 2021 by qweryy

> Written by 范棋珈, modified by panic

### 工具

#### 1. 字符串



### 存储模块

用来存储用户和书本。由于对日志的索引要求较低，可以直接顺序存储。

```c++
template <BptStorable KeyType, BptStorable ValueType, size_t szChunk>
class BpTree {
 public:
  BpTree () = delete;
  BpTree (const char *filename);
  void add (const KeyType &key, const ValueType &value);
  void del (const KeyType &key, const ValueType &value);
  // 检查树中是否有 (key, value)
  bool find (const KeyType &key, const ValueType &value);
  void query (const KeyType &key, std::vector<ValueType> &store);
};
```

### 节点模块

#### 1. 用户类

用户拥有 ID（唯一）、名字、权限和密码。使用 ID 来重载小于号。

```c++
enum Privilege { kGuest = 0, kCustomer = 1, kWorker = 3, kRoot = 7 };

class User {
 private:
  // 不能用 std::string，因为 string 的实际内容是在堆内存里的
  ak::file::Varchar<30> id_, name_, password_;
  Privilege privilege_;

 public:
  // TODO : 构造函数

  const std::string &id () { return id_; }
  const std::string &name () { return name_; }
  const std::string &password () { return password_; }

  bool operator< (const User &rhs) const {  // 根据 id 排序
    return id < rhs.id;
  }
  void passwd (const std::string &newPassword);  // 修改密码
};
```

#### 2. 基类：书类

每本书有 ISBN（唯一）、书名、作者名、关键词、数量、单价。

基类以 isbn 为排序关键字。

```c++
class Book {
 public:
  ak::file::Varchar<20> isbn;
  ak::file::Varchar<60> name, author, keyword;
  long long quantity;
  long long price;

  Book() = default;
  bool operator< (const Book &rhs) const {
    return isbn < rhs.isbn;
  }
  // 修改与进货等直接访问成员变量。
  // TODO : 一些操作函数。
};
```

##### 2.1 派生：索引为关键词的书类

以二元组 (keyword, isbn) 为排序关键字。

按照 Keyword 索引时注意要把原来的书按关键词拆分（注意到 `show` 指令只需要查某一个关键词对应的图书）。

```c++
class Keyword_Book : public Book {
 public:
  Keyword_Book(const Book &obj) : Book(obj) {}
  bool operator<(const Keyword_Book &rhs) const {
    if (keyword == rhs.keyword) return isbn < rhs.isbn;
		return keyword < rhs.keyword;
  }
};
```

##### 2.2 派生：索引为作者的书类

```c++
class Author_Book : public Book;
```

##### 2.3 派生：索引为书名的书类

```c++
class Name_Book : public Book;
```

这两个派生类实现与 **2.1** 类似。

#### 3. 日志

### 节点管理模块

#### 1. 用户管理类

实现用户有关的命令与登录栈。

```c++
class UserManager {
 private:
  // key 为 user id
  BpTree<ak::file::Varchar<30>, User> users;
  // pair 的 second 为选中的 book id, 初始为 -1
  std::vector<std::pair<User, int>> userStack_;
  User &currentUser_ ();

 public:
  UserManager () = delete;
  // 这里应该初始化 userStack_ 为只有一个匿名帐号
  UserManager (const char *filename);
  void logIn (const std::string &id, const std::string &password = "");
  void logOut ();
  void signUp (const std::string &id, const std::string &password, const std::string &name);
  void userAdd (const std::string &id, const std::string &password, Privilege p, const std::string &name);
  void remove (const std::string &id);
};
```

#### 2. 图书管理类

实现图书有关的命令。

```c++
class BookManager {
  enum ShowType { kIsbn, kKeyword, kAuthor, kName };
 private:
  BlockList<Book, 400> books;
  BlockList<Keyword_Book, 400> keyword_books;
  BlockList<Author_Book, 400> author_books;
  BlockList<Name_Book, 400> name_books;

 public:
  void show (ShowType type, const std::string &s);
  void buy (const std::string &isbn, const int &cnt);
  void select (const std::string &isbn);
  // 对于某本书的操作在 Book 类的成员函数内。
};
```

#### 3. 日志管理类

目前想法是在文件中顺序存储。

### 文件结构

#### 1. 数据文件

1. books.dat
2. keyword_books.dat
3. author_books.dat
4. name_books.dat
5. users.dat
6. user_stack.dat
7. log.dat

#### 2. 索引文件

#### 3. 代码文件

1. main.cpp 主入口，解析命令与调用相应的类
2. blocklist.h/cpp 数据结构的头文件和实现文件
3. users.h/cpp 实现用户类与用户管理类。
4. books.h/cpp 实现书类和管理类
5. logs.h/cpp 日志管理类
