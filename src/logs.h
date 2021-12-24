#ifndef PANIC_BOOKSTORE_LOGS_H_
#define PANIC_BOOKSTORE_LOGS_H_

#include <ak/file/file.h>
#include <ak/file/varchar.h>
#include <string>

class TradeRecord {
 private:
  // ak::file::Varchar<30> id_
  bool isExpense_;
  long long income_ = 0, expense_ = 0;

 public:
  // 构造函数，type = 0 为收入，= 1 为支出。
  TradeRecord (const bool &type, long long amount);
  // 支持多笔交易记录相加。
  TradeRecord &operator+= (const TradeRecord &);
  // 按照题目要求格式输出。
  friend std::ostream &operator<< (std::ostream &, const TradeRecord &);
};

class CmdRecord {
 private:
  ak::file::Varchar<30> userId_;  // 执行者
  ak::file::Varchar<1024> command_;  // 原始命令

 public:
  CmdRecord (const std::string &, const std::string &);  // 构造函数。
  friend std::ostream &operator<< (std::ostream &, const CmdRecord &);  // 输出重载。
};

class LogManager {
 private:
  ak::file::File<sizeof(TradeRecord)> tradeFile_;
  ak::file::File<sizeof(CmdRecord)> cmdFile_;

  // 私有成员函数，读取文件开头存的记录数量。
  int tradeCount_ ();
  int cmdCount_ ();

 public:
  // 初始化，文件名为 name + "_trade.bin"/"_cmd.bin".
  // 注意，每个文件开头预留一个 int 存储交易记录/命令记录的数量。
  LogManager (const std::string &name);
  // 在文件末尾加入一个交易记录，并修改交易记录数量。
  void addTrade (const TradeRecord &);
  // 对应题目命令，计算后 cnt 条交易记录并输出。
  void showFinance (int cnt);
  void showFinance ();
  // 输出所有交易记录。
  void reportFinance ();
  // 在文件末尾加入一个命令记录，并修改命令记录数量。
  void addLog (const CmdRecord &);
  // 对应题目命令 report myself，从头到尾查找并输出某个员工的命令记录。
  // 对于命令 report employee，使用上述默认参数可以匹配所有 id 的记录。
  void reportEmployee (const std::string &id_ = "");
  // 可以自由决定实现方式，或者可以分别调用 ReportFinance() 和 ReportEmployee().
  void reportLog ();

  void clearCache ();
};

#endif
