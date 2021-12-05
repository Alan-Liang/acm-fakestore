Bookstore 2021 技术规格书 by Alan-Liang
=======================================

产品规格书见 <https://github.com/ACM-Programming-2021/Bookstore-2021/blob/main/README.md>。

[TOC]

## 项目结构

<details>
<summary>目录树</summary>

标 `*` 的文件是可执行文件; 标 `?` 的目录及文件可以视自己情况实现或不实现; `#` 及以后的内容为注释; `ull.{cpp,h}` 可以替换为 `bptree.{cpp,h}`。

```bash
$ tree -avF --noreport bookstore/
bookstore/         # 项目根目录
├── .clang-format? # 自动格式化配置
├── .editorconfig? # 代码格式配置
├── .git/
│   └── ...
├── .gitignore
├── CMakeLists.txt
├── LICENSE
├── README.md
├── code*
├── data/          # 运行时数据库
│   ├── .gitkeep                  # 用来使 git 保留这个目录
│   ├── accounts                  # 帐户信息
│   ├── accounts.user-id.ix       # 帐户关于 User ID 的索引
│   ├── books                     # 图书信息
│   ├── books.author.ix           # 图书关于作者的索引
│   ├── books.isbn.ix             # 图书关于内部奇怪图书编号的索引
│   ├── books.name.ix             # 图书关于书名的索引
│   ├── keywords                  # 关键词；详细描述见下
│   ├── keywords.book-id.ix       # 关键词关于图书 ID 的索引
│   ├── keywords.keyword.ix       # 关键词关于关键词本身的索引
│   └── transactions              # 交易信息
├── docs/          # 文档
│   ├── design.md          # 本文档
│   ├── implementation.md? # 关于实现细节的文档
│   └── manual.md?         # 用户手册
├── lib/           # 一些与业务逻辑关系不大的通用代码
│   ├── model.cpp    # 存储对象的抽象和维护；详情见下
│   ├── model.h
│   ├── store.cpp    # 对象存储
│   ├── store.h
│   ├── ull.cpp      # 块链
│   ├── ull.h
│   ├── varchar.cpp  # 字符串存储
│   └── varchar.h
├── logs/          # 日志文件
│   ├── .gitkeep
│   └── bookstore.log
├── scripts/?      # 开发时使用的脚本
│   ├── build*     # 构建项目
│   ├── clean*     # 清理项目构建和数据库
│   ├── test*      # 运行测试
│   └── truncate*  # 清空数据库
├── src/           # 与业务逻辑相关的代码
│   ├── commands.cpp # 指令处理与执行
│   ├── commands.h
│   ├── logging.cpp  # 日志的写入与读取
│   ├── logging.h
│   ├── main.cpp     # 入口点
│   ├── models/      # 对象模型；详细描述见下
│   │   ├── account.cpp     # 帐户
│   │   ├── account.h
│   │   ├── book.cpp        # 图书和关键词
│   │   ├── book.h
│   │   ├── transaction.cpp # 交易
│   │   └── transaction.h
│   ├── models.h     # 用来 include 所有模型的工具头文件
│   ├── utils.cpp    # 各种工具类和函数
│   └── utils.h
└── testpoints/    # 测试点
    └── ...
```

</details>

<details>
<summary><code>.gitignore</code> 文件内容</summary>

```plain
# https://github.com/github/gitignore/blob/master/CMake.gitignore
CMakeLists.txt.user
CMakeCache.txt
CMakeFiles
CMakeScripts
Testing
Makefile
cmake_install.cmake
install_manifest.txt
compile_commands.json
CTestTestfile.cmake
_deps

data/
!data/.gitkeep
testpoints/
**/*.o
**/*.log

.vscode/
.idea/
cmake-build-debug/

code
```

</details>

<details>
<summary>快速创建目录结构</summary>

只需一行命令：

```bash
echo '/Td6WFoAAATm1rRGAgAhARYAAAB0L+Wj4Av/AtldAGOcPC/kRxBdYCwEY+vd4Ll8OFi7V3Tqb3mAqk4/N983cgdSWsx9uyuSQ8h8DQf3q0jIv/5I9ee4/iMqe4S32y2LPyI48rIgZaAaBBgV/pP1pfg1VRn8pHCxGjGyh5o3dch5fMXUMLWbjvpuDF8FyjBPzA/UZIFQPu8pthGfZCRGQFU5rP9FWrFo1/d7UF2O/VyMkR1we+/uWdicP72fXHFeiSyC94LYGA+/9H5pLKDYIBt5UMg+Js4BiKX49488nNAfhaywfX/rWDmyCS3B8TnOmmBV5xRmAy4qPjPj8ZfUNligeDKw5LTzUag2u658Ki8osrV+TTXTcFwCfFNuDFyHRxqQuN1rgX8w+BGdJ2GOLVzzcShk6nYhcj7SCBvOAF+kZyYFShYLBg8XfdwQ0KKxlggPPgUDKOlctlOeiDMkaBkQ4iw8v0hpWYTaxBz7J3i0SqvnxOOsiQYlOmNTUPgjAGB6hvzeABTwuzbC7qm1PPFOuPttVOXhkP8pPiRQl2qrePSHDH7EnUpLaXqXz4W79/36GVMKcYirTCvYSXYYChC6GFX4+ZqAwpVGAB+bTogtnd5grf1NLd9J2ghmIT8SUjExtT1KrIPmnVCRh2dKLMe4jk8vOZjl7nc+hCfTYTilimB3g59Qm6Mn0RxEPy6djAVP7ZiY4FmImD/rSn6ZGqZ1iDu3AHHyNV0Od++AB4GEEf0x+dttrqBbCLhqA6LIV1hfaMcJCkrKuX+uWXQzqoR26bXfLI/VjVryo4Rj+LU1mVyQHOpN89p0h56rcmYR9FRKwKbFiYYTmbCKA9n5Piiga26nsL5abtWmj/duGqk9xQyZGFA0V1jYtFFposI8jiIjfEBIafv56YIWa57Vs1K9i3wFEtOp7rva56xcSGVrx7OucIyew2iIngFcln3tN7NlKYz5WYGeAhZZNckjoNNvmdbrkTD02PDZgH4YKnpiEpT901ZUAAAAAABYYrjQcQVQLQAB9QWAGAAA9JeGsbHEZ/sCAAAAAARZWg==' | base64 -d | xz -d | cpio -i
```

</details>

## 存储结构

数据的实际内容存储在对应的数据文件中，由 `Store` 类管理。每个数据条目（如一个帐户、一本图书、一条交易记录）都有一个自然数 id，也存储在数据文件中。（换句话说，数据文件中存放的是一个自然数 id 和对应的所有数据。）以帐户为例，数据文件可能会长这样：（不一定要按照这个格式实现）

```plain
# accounts
0300 0000 # (int) 3, 代表数据条目个数

# 第一条记录
0000 0000 # (int) 0, 自然数 id
726f 6f74 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00 # (char[31]) root，用户 ID
736a 7475 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00 # (char[31]) sjtu，密码
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00 # (char[31]) (空字符串)，用户 name
00 0000 # 意义不明的 padding，由 C++ class 的内部存储结构引入，下同
0700 0000 # (Privilege) 7，代表店主身份

# 第二条记录
0100 0000 # (int) 1, 自然数 id
776f 726b 6572 6964 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00 # (char[31]) workerid，用户 ID
7061 7373 776f 7264 5f77 6f72 6b65 7200 0000 0000 0000 0000 0000 0000 0000 00 # (char[31]) password_worker，密码
776f 726b 6572 6e61 6d65 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00 # (char[31]) workername，用户 name
00 0000 # 意义不明的 padding
0300 0000 # (Privilege) 3，代表员工身份

# 第三条记录
ffff ffff # (int) -1，代表此条数据被删除，此块可被回收再利用
776f 726b 6572 3200 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00 # (char[31]) worker2，用户 ID
7061 7373 776f 7264 5f77 6f72 6b65 7232 0000 0000 0000 0000 0000 0000 0000 00 # (char[31]) password_worker2，密码
776f 726b 6572 6e61 6d65 3200 0000 0000 0000 0000 0000 0000 0000 0000 0000 00 # (char[31]) workername2，用户 name
00 0000 # 意义不明的 padding
0300 0000 # (Privilege) 3，代表员工身份
```

为了实现 `show` 等命令的快速查找，我们需要对数据建立索引，记录在索引文件里。索引文件由块状链表或 B+ 树存储在 `.ix` 文件里。块链或 B+ 树的 key 为需要被索引的字段（如 `Account::name` 或 `Book::isbn`），值为对应记录的自然数 id。例如要对上面两条记录建立关于 `userId` 的索引，则应有：

```cpp
// accounts.user-id.ix
index["root"] -> 0
index["workerid"] -> 1
/* 不应该有:
 * index["worker2"] -> -1
 * 或者
 * index["worker2"] -> 2 */
```

## 代码结构

### 工具库

#### 字符串存储 `varchar.{cpp,h}`

实现一个 `char [n]` 的包装。当输入的 `std::string`, `const char *` 或者 `Varchar<A>` 长度超过 `maxLength` 时抛出异常。

这个类存在的意义在于，如果用 `char []` 存数据，则无法方便地实现等号赋值和比较，需要特判；如果用 `std::string` 则无法方便地直接存储到文件中（因为 `std::string` 的实际内存在堆空间中），也需要特判。而使用 `Varchar<>` 存数据，则既可以 `user->name = "root";` 也可以直接 `write(static_cast<char *>(&data), sizeof(data));`。

```cpp
template <int maxLength>
struct Varchar {
 private:
  // +1 是因为要存结尾 '\0'
  char content[maxLength + 1];
 public:
  // 默认为空字符串
  Varchar ();
  Varchar (const std::string &s);
  Varchar (const char *cstr);
  operator std::string () const;
  std::string str () const;
  // 赋值操作
  template <int A>
  Varchar operator= (const Varchar<A> &that);
  // 比较: https://en.cppreference.com/w/cpp/language/default_comparisons
  template <int A>
  std::weak_ordering operator<=> (const Varchar<A> &that) const;
  template <int A>
  bool operator== (const Varchar<A> &that) const;
  template <int A>
  bool operator!= (const Varchar<A> &that) const;
};
```

#### 分块存储 `store.{cpp,h}`

实现一个类似于 Memory River 的分块存储结构，需要支持一个数值索引 `id`，并保证在没有 `remove` 操作的情况下 `id` 非负且随时间递增。用于存储实际数据。

```cpp
template <typename PayloadType>
class Store {
 private:
  int nextId_; // 下一个产生的 id
 public:
  Store (const char *filename); // 初始化文件存储
  PayloadType select (int id); // 查找一条记录
  std::set<PayloadType> selectLimit (int limit); // 查找最后 limit 条记录
  std::set<PayloadType> selectAll (); // 返回所有记录
  int insert (const PayloadType &payload); // 添加一条记录
  void update (int id, const PayloadType &payload); // 更新一条记录
  void remove (int id); // 删除一条记录
};
```

#### 块状链表 `ull.{cpp,h}`

实现一个块状链表 `UnrolledLinkedList<KeyType, ValueType>`，支持以下四种操作:

```cpp
template <typename KeyType, typename ValueType>
class UnrolledLinkedList {
 public:
  UnrolledLinkedList (const char *filename);
  void add (const KeyType &key, const ValueType &value);
  void remove (const KeyType &index, const ValueType &value);
  std::optional<ValueType> findOne (const KeyType &index);
  std::vector<ValueType> findMany (const KeyType &index);
};
```

其中 `KeyType` 可比较。用于存储索引。

#### 模型 `model.{cpp,h}`

实现一个简单的 [ORM][orm] 来简化对象存储。每一个 `Model` 的派生类对应着一个存储数据用的*表*。使用方法示例:

```cpp
#include <cstring>
#include "../lib/model.h"

class College : public Model<College> {
 public:
  Varchar<100> name;
  Varchar<11> abbrev;
  int rank;
  College () = default;
  College (std::string name, std::string abbrev, int rank) : name(name), abbrev(abbrev), rank(rank) {}
  void check () {
    if (rank < 0) throw std::exception(/* TODO */);
    if (name == "") throw std::exception(/* TODO */);
  }
 private:
  // 这个对象会被 Model 用到, 因此重命名时请保证两处命名一致
  static Manager manager_;
  // Model 会用到我们的 manager_, 所以需要声明为 friend
  friend class Model<College>;
};
College::Manager College::manager_ {
  /* filename */ "./data/colleges",
  /* indices */ {
    { &College::abbrev, "./data/college.abbrev.ix" },
    { &College::rank, "./data/college.rank.ix" },
  },
};

College("Peking University", "PKU", 1).save();
College("Tsinghua University", "THU", 1).save();
College("Shanghai Jiao Tong University", "SJTU", 3).save();
College("Fudan University", "FDU", 3).save();

auto thu = College::findOne(&College::abbrev, "THU");
thu->rank = 2;
thu->update();

std::cout << College::findOne(&College::abbrev, "FDU")->rank << std::endl; // 3
std::cout << College::findOne(&College::rank, 1)->abbrev << std::endl; // PKU
for (const auto &college : College::findMany(&College::rank, 3)) {
  std::cout << college.abbrev << std::endl; // SJTU FDU (in that order)
}

College::findOne(&College::abbrev, "FDU")->remove();
for (const auto &college : College::findMany(&College::rank, 3)) {
  std::cout << college.abbrev << std::endl; // SJTU
}
```

至少需要的原型:

```cpp
template <typename M>
class Model {
 public:
  int id;
  // check() 后将从未保存过的 Model 插入 M::manager_.store_ 并创建索引, 若已经保存过则 throw, 并把 id 赋值为 Store 返回的 id
  // Implementation notes: 可以用 id == -1 来表明没有保存过
  void save ();
  // check() 后更新一个已经保存过的 Model 并更新索引, 若从未保存则 throw
  void update () const;
  // 删除一个已经保存过的 Model 和它的索引, 并将 id 重置, 若从未保存则 throw
  void remove ();
  // 默认行为为比较 id, 派生类可重载成 ISBN 等的比较
  friend bool operator< (const Model &lhs, const Model &rhs);
  // 检查当前对象是否有问题，有则抛出异常。默认什么都不干
  virtual void check ();

  // 找出特定 id 的 Model 对象, 不存在则返回 std::nullopt, 下同
  static std::optional<M> findById (int id);
  // 找出第一个 key 为 value 的 Model 对象, 需要在这个 key 上有索引; 需要对 const char * 特化, 下同
  template <typename ValueType>
  static std::optional<M> findOne (ValueType M::*key, ValueType value);
  // 需要对 varchar 特判一下
  template <int A> static std::optional<M> findOne (Varchar<A> M::*key, std::string value);
  // 找出所有 key 为 value 的 Model 对象, 需要在这个 key 上有索引
  template <typename ValueType>
  static std::set<M> findMany (ValueType M::*key, ValueType value);
  template <int A> static std::set<M> findMany (Varchar<A> M::*key, std::string value);
  // 找出 id 最大的 n 个 Model 对象, 需要在这个 key 上有索引
  static std::set<M> findAll (int limit);

 protected:
  // 初始化信息（本项目中只在 Account 中用到）
  static void init_ () {}
  // 临时对象, 用来传入索引信息
  struct Index {
    /*
      key 在 Model 上的 offset
      Implementation notes:
      #define MODEL_OFFSETOF(x) ((int)(&(((M *)0)->*x)))
      Index (...) : offset(MODEL_OFFSETOF(*key)), filename(filename) {}
    */
    int offset;
    // 存索引的文件名
    const char *filename;
    // 通过 member pointer 快速构造
    template <typename ValueType>
    Index (ValueType M::*key, const char *filename);
  };
  class Manager {
   private:
    // 索引列表, key 是 offset; 每个索引是一个块链或 B+ 树
    std::unordered_map<int, const char *> indices_;
    Store<M> store_;
   public:
    Manager () = delete;
    // 在这里读入或初始化数据文件，若数据文件不存在则需要调用 M::init_()
    Manager (const char *filename, std::initializer_list<Index> indices);
    // 获取指定 key 的索引, 不存在索引则 throw
    template <typename ValueType>
    UnrolledLinkedList<ValueType, int> getIndex (ValueType M::*key);
  };
};
```

里面用到了 `std::optional` 和 member pointer 两个特性, 可以参考 [这份代码][memptr-eg] 和 [C++ Reference 上的这篇关于 optional 的文章][optional-cppref]。

[orm]: http://en.wikipedia.org/wiki/Object%E2%80%93relational_mapping
[memptr-eg]: https://paste.sr.ht/~alan-liang/85ff4acd6b442bf30ad79ad91f03e9fadaa918b8
[optional-cppref]: https://en.cppreference.com/w/cpp/utility/optional

##### 为什么要搞这么个东西？~~不是没事找事吗~~

对比一下两段代码:

```cpp
// https://github.com/PaperL/BookStore_SiriusNEO/blob/16447e454d68ddefb34a91a26379e3287256b2d2/usermanager.cpp#L281-L297 ，有简化
vector<int> tempVec;
id_cmd.findNode(id, tempVec);//查找是否有该用户
if (tempVec.empty()) {//没有该账户
  printf("Invalid\n");
  return;
}
int offset;
User tempUser;
string temps;

offset = tempVec[0];
tempUser = freadUser(offset);
temps=tempUser.passwd;
```

和

```cpp
auto tempUser = Account::findOne(&Account::userId, id);
if (!tempUser) {//没有该账户
  printf("Invalid\n");
  return;
}
std::string temps = tempUser->password;
```

显然后者能更简洁清晰地表现出这段代码的意图，并且减少了大量重复代码（`offset`, `tempVec`, `freadUser` 等）。这样的机制还有一些其他好处：

- 文件的具体存储结构和业务逻辑里的表示分离。想给所有数据加密？只需要改 `Model` 类就可以了。
- 容易增加新的存储结构。想加个文件存失败的登录请求？直接 `class FailedLogInAttempt : public Model<FailedLogInAttempt> {...};` 就行。
- ~~与实际工程接轨~~

### 业务逻辑相关

#### 对象模型 `models/`

##### 帐户 `account.{cpp,h}`

```cpp
class Account : public Model<Account> {
 public:
  enum Privilege { PRIVILEGE_ANONYMOUS = 0, PRIVILEGE_CUSTOMER = 1, PRIVILEGE_WORKER = 3, PRIVILEGE_ROOT = 7 };
  Varchar<30> userId;
  // 这里明文存储密码; 实际工程中不应如此操作
  Varchar<30> password;
  Varchar<30> name;
  Privilege privilege;
  void check ();
 protected:
  // 初始化 root 帐户
  static void init_ ();
 private:
  static Manager manager_;
  friend class Model<Account>;
};
Account::Manager Account::manager_ {
  "./data/accounts",
  {
    { &Account::userId, "./data/accounts.user-id.ix" },
  },
};
```

##### 图书及关键词 `book.{cpp,h}`

```cpp
class Book : public Model<Book> {
 public:
  static constexpr int KEYWORDS_MAX_LENGTH = 60;
  Varchar<20> isbn;
  Varchar<60> name;
  Varchar<60> author;
  int stock; // 存货数量
  unsigned long long priceCent; // 按分记录的价格
  void check ();
  bool operator< (const Book &that) const; // Book 按 ISBN 排序
  // 按题目要求输出图书信息
  void printf () const;
  void setKeywords (const std::vector<std::string> &keywords);
  // implementation notes: Keyword::findMany(&Keyword::bookId, id);
  std::set<std::string> getKeywords () const;
  // 找出所有含有这个关键词的 Book
  static std::set<Book> findByKeyword (const std::string &keyword);
 private:
  static Manager manager_;
  friend class Model<Book>;
};
Book::Manager Book::manager_ {
  "./data/books",
  {
    { &Book::isbn, "./data/books.isbn.ix" },
    { &Book::author, "./data/books.author.ix" },
    { &Book::name, "./data/books.name.ix" },
  },
};

class Keyword : public Model<Keyword> {
 public:
  int bookId;
  Varchar<60> keyword;
  void check ();
 private:
  static Manager manager_;
  friend class Model<Keyword>;
};
Keyword::Manager Keyword::manager_ {
  "./data/keywords",
  {
    { &Keyword::bookId, "./data/keywords.book-id.ix" },
    { &Keyword::keyword, "./data/keywords.keyword.ix" },
  },
};
```

##### 交易 `transaction.{cpp,h}`

```cpp
class Transaction : public Model<Transaction> {
 public:
  static constexpr int MAX_QUANTITY = 2147483647;
  enum TransactionType { TX_BUY, TX_IMPORT };
  TransactionType type;
  int bookId;
  int quantity;
  unsigned long long amountCent; // 正数, 单位为分
  void check ();
  Book getBook () const;
 private:
  static Manager manager_;
  friend class Model<Transaction>;
};
Transaction::Manager Transaction::manager_ {"./data/transactions", {}};
```

#### 导入所有模型 `models.h`

```cpp
#include "models/account.h"
#include "models/book.h"
#include "models/transaction.h"
```

#### 日志的写入与读取 `logging.{cpp,h}`

除 `select` 外，对于每一条成功解析并执行的命令，需要调用这里的接口留下日志。日志以纯文本文件写入在 `logs/bookstore.log` 中。特别的，密码错误的登录请求和更改密码请求也需记录。

除 `modify` 外，日志格式为: `<time> <userId> <result> <command>`，其中 `<time>` 为 UNIX 时间戳，`<result>` 为 `success` 或 `fail`。匿名用户的 `<userId>` 记录为 `<anon>` (一个不合法的 user id)。`modify` 的格式为 `<time> <userId> <result> modify <isbn> <update>+`。例如:

```plain
1638601146 <anon> fail su root 123456
1638601146 <anon> success su root sjtu
1638601146 root success modify 978-7-19260817 -name="sjtu"
1638601147 root success logout
1638601147 <anon> success quit
```

被要求输出时程序读入并解析这个文件。

```cpp
constexpr const char *logfile = "./logs/bookstore.log";
void writeLog (const std::string &command);
void printLog (void);
void printReport (const Account &account);
void printFinanceReport (void);
void printEmployeeReport (void);
```

#### 指令处理与执行 `commands.{cpp,h}`

为了防止与语言关键词冲突，`register`, `delete`, `import` 三个函数名后加了下划线。

```cpp
namespace command {
class Shell {
 public:
  std::optional<Account> account;
  // 当前选中的 book id
  int selectedBookId = -1;
  // 登录状态
  static std::vector<Shell> stack;
  // 返回 stack 顶的 shell, 即 Shell::stack.back()
  static Shell &current ();
  // 在当前 account 权限等级严格小于 level 时 throw
  static void requestPrivilege (Account::Privilege level);
};
std::vector<Shell> Shell::stack = { {} };

void quit (void);
void exit (void);

void su (void);
void logout (void);
void register_ (void);
void passwd (void);
void useradd (void);
void delete_ (void);

void showBook (void);
void buy (void);
void select (void);
void modify (void);
void import_ (void);

void report (void);
void showFinance (void);
void log (void);
} // namespace command
```

#### 工具类和函数 `utils.{cpp,h}`

在这里定义其他代码中用到的工具类和函数（例如，验证一个输入的浮点数的格式并转换为按分计算的整数）。

#### 入口点 `main.cpp`

按行读入并解析指令。

```cpp
int main (void);
```

## 杂七杂八的文件

### `.clang-format`

用来定义代码格式的文件，可以参考 [Clang 的官方文档][cl-format]。

[cl-format]: https://clang.llvm.org/docs/ClangFormat.html

### `.editorconfig`

定义代码的基本格式规范，可以参考 [官方网站][editorconfig]。

[editorconfig]: https://editorconfig.org/

## Metadata

| Authors    | Version |
|:-----------|:--------|
| Alan-Liang | 2       |

### 如何修改本文档

1. 请把代码规范调整为您（实现者）使用的代码规范。特别需要注意的地方：文件名（`.cpp` 还是 `.cc`？），命名格式（`PascalCase`, `camelCase` 还是 `snake_case`？），命名规范（用不用 Hungarian Naming Convention？）
2. 请修改 `data/` 目录下的文件结构来符合您的实现
3. 请删除您不打算实现的部分和相关说明
4. 更改本文档时请同步更新上面表格里的 Authors 和 Version
