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

  std::string id () { return id_; }
  std::string name () { return name_; }
  std::string password () { return password_; }
  Privilege privilege () { return privilege_; }

  bool operator< (const User &rhs) const {  // 根据 id 排序
    return id_ < rhs.id_;
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

#### 3. 日志

日志有两种，一种是交易记录，一种是命令记录。

##### (1) 交易记录

记录每笔交易的收入、支出（单笔交易至少有一个量为 0）、交易用户 id（如果需要），支持加法、输出。

```c++
class TradeRecord {
 private:
  // ak::file::Varchar<30> id_
  double income, expense;

 public:
  // 构造函数，type = 0 为收入，= 1 为支出。
  TradeRecord(const bool &type, const double &);
  // 支持多笔交易记录相加。
  TradeRecord &operator+=(const TradeRecord &);
  // 按照题目要求格式输出。
  friend std::ostream &operator<<(std::ostream &, const TradeRecord &);
};
```

##### (2) 命令记录

记录每个指令的执行者与内容。

```c++
class CmdRecord {
 private:
  ak::file::Varchar<30> user_id;  // 执行者
  ak::file::Varchar<1024> command;  // 原始命令
  int number;  // 表示命令编号。

 public:
  CmdRecord(const std::string &, const std::string &);
  friend std::ostream &operator<<(std::ostream &, const CmdRecord &);  // 输出重载。
};
```



### 节点管理模块

#### 1. 用户管理类

实现用户有关的命令与登录栈。

```c++
class UserManager {
 private:
  // key 为 user id
  BpTree<ak::file::Varchar<30>, User> users_;
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
  void passwd (const std::string &id, const std::string &current, const std::string &newPassword);
  void remove (const std::string &id);
};
```

#### 2. 图书管理类

实现图书有关的命令。

```c++
class BookManager {
  enum Field { kIsbn, kKeyword, kAuthor, kName, kPrice };
 private:
  BpTree<ak::file::Varchar<20>, Book> books_;
  BpTree<ak::file::Varchar<60>, ak::file::Varchar<20>> nameBooks_;
  BpTree<ak::file::Varchar<60>, ak::file::Varchar<20>> keywordBooks_;
  BpTree<ak::file::Varchar<60>, ak::file::Varchar<20>> authorBooks_;

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
  // 对于某本书的操作在 Book 类的成员函数内。
};
```

#### 3. 日志管理类

在这个类中实现存储两种记录。直接在文件中按顺序存储（或用 B+ Tree 代为存储）。

```c++
class LogManager {
 private:
  std::string trade_file_name;
  std::string cmd_file_name;
  // 也可以使用 B+ Tree 代为存储。
  // int sizeof_trade, sizeof_cmd;

  // 私有成员函数，读取文件开头存的记录数量。
  const int TradeCount();
  const int CmdCount();

 public:
  void Init(const std::string &name);
  // 初始化，文件名为 name + "_trade.bin"/"_cmd.bin".
  // 注意，每个文件开头预留一个 int 存储交易记录/命令记录的数量。
  void AddTrade(const TradeRecord &);
  // 在文件末尾加入一个交易记录，并修改交易记录数量。
  void ShowFinance(const int &cnt);
  // 对应题目命令，计算后 cnt 条交易记录并输出。
  void ReportFinance();
  // 输出所有交易记录。
  void AddLog(const CmdRecord &);
  // 在文件末尾加入一个命令记录，并修改命令记录数量。
  void ReportEmployee(const std::string &id_ = "");
  // 对应题目命令 report myself，从头到尾查找并输出某个员工的命令记录。
  // 对于命令 report employee，使用上述默认参数可以匹配所有 id 的记录。
  void ReportLog();
  // 可以自由决定实现方式，或者可以分别调用 ReportFinance() 和 ReportEmployee().
};
```



### 文件结构

#### 1. 数据文件

1. books.dat
1. keyword_books.dat
1. author_books.dat
1. name_books.dat
1. users.dat
1. log.dat

#### 2. 索引文件

#### 3. 代码文件

1. main.cpp 主入口，解析命令与调用相应的类
2. bptree.h 数据结构的头文件和实现文件
3. users.h/cpp 实现用户类与用户管理类。
4. books.h/cpp 实现书类和管理类
5. logs.h/cpp 日志管理类
