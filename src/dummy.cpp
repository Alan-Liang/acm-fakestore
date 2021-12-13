#include <ak/base.h>
#include <ak/file/bptree.h>
#include <ak/file/varchar.h>
#include <iostream>

using ak::file::Varchar;

int main () {
  ak::file::BPTree<Varchar<64>, int> bpt {"file"};
}
