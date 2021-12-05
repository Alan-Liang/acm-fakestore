# BookStore 2021 by qweryy

> Written by 范棋珈

### 工具

#### 1. 字符串



### 存储模块

用来存储用户和书本。由于对日志的索引要求较低，可以直接顺序存储。

> 注：若需要实现 B+ Tree 可以根据实际情况自行修改。

#### 1.  块类

每个块内存一个数组，模板为数组元素类和块大小。

按照 `NodeType` 重载的小于号升序排序。 

```c++
template <class NodeType, size_t SIZ>
class Block {
 private:
  std::string file_name;
  Block *nxt, *lst;
  NodeType array[SIZ];

 public:
  void add(const NodeType &obj);
  void del(const NodeType &obj);
  Block &Merge(Block &obj);
  void split();
};
```

#### 2. 块状链表类

本质是一个链表，把所有块链起来。

按照 `NodeType` 重载的小于号升序排序。 

```c++
template <class NodeType, size_t SIZ>
class BlockList {
 protected:
  std::string file_name;
  Block<NodeType, SIZ> *head = nullptr, *tail = nullptr;

 public:
  BlockList() = default;
  BlockList(const std::string &file_name);
  // 根据索引文件读入每个 Block.
  void add(const NodeType &obj);
  void del(const NodeType &obj);
};
```

### 节点模块

#### 1. 用户类

用户拥有 ID（唯一）、名字、权限和密码。使用 ID 来重载小于号。

```c++
enum Privilege { kGuest = 0, kCustomer = 1, kWorker = 3, kRoot = 7 };

class User {
 private:
  std::string id, name, password;
  Privilege privilege;

  // TODO : 构造函数

  bool operator<(const User &rhs) const {  // 根据 id 排序
    return id < rhs.id;
  }
  void Passwd(const std::string &new_password);  // 修改密码
};
```

#### 2. 基类：书类

每本书有 ISBN（唯一）、书名、作者名、关键词、数量、单价。

基类以 isbn 为排序关键字。

```c++
class Book {
 public:
  std::string isbn, name, author, keyword;
  int quantity, price;

  Book() = default;
  bool operator<(const Book &rhs) const {
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
  BlockList<User, 400> users;  // 数字可修改。
  User current_user;
  std::string user_stack_file;

 public:
  void LogIn(const std::string &id, const std::string &password = "");
  void LogOut();
  void Register(const std::string &id, const std::string &password, const std::string &name);
  void UserAdd(const std::string &id, const std::string &password, Privilege p, const std::string &name);
  void Delete(const std::string &id);
};
```

#### 2. 图书管理类

实现图书有关的命令。

```c++
class BookManager {
  enum ShowType { kISBN, kKeyword, kAuthor, kName };
 private:
  BlockList<Book, 400> books;
  BlockList<Keyword_Book, 400> keyword_books;
  BlockList<Author_Book, 400> author_books;
  BlockList<Name_Book, 400> name_books;

 public:
  Book current_book;
  void Show(ShowType type, const std::string &s);
  void Buy(const std::string &isbn, const int &cnt);
  void Select(const std::string &isbn);
  // 对于某本书的操作在 Book 类的成员函数内。
};
```

#### 3. 日志管理类

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
