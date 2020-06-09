#include <iostream>
#include <sysexits.h>

#include "lexer.hh"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: dukkha <file.du>\n";
    return EX_USAGE;
  }

  const char* file_path = argv[1];

  Lexer lexer;
  if (!lexer.from_file(file_path)) {
    return EX_NOINPUT;
  }

  Token token = lexer.next();
  while (token.type != TokenType::EndOfFile) {
    std::cout << token.type << "\n";

    if (token.type == TokenType::Identifer) {
      std::cout << "  " << token.as_string << "\n";
    }

    token = lexer.next();
  }

  std::cout << "\n";

  return EX_OK;
}
