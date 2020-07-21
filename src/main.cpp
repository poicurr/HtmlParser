#include <fstream>
#include <iostream>
#include <string>

#include "Parser.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " /path/to/html" << std::endl;
    return 1;
  }

  std::string s = {};
  char buf[1024];
  std::ifstream ifs(argv[1]);
  while (!ifs.eof()) {
    ifs.read(buf, 1024);
    s.append(buf, ifs.gcount());
  }

  const char* p = s.c_str();
  TokenList tokenList = tokenize(p);
  TagNode* document = parse(tokenList);

  std::cout << " --- parsed html structure --- " << std::endl;
  for (Node* node : document->children) {
    node->print("");
  }
}
