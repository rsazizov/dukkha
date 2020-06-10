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
    std::cout << token.type
              << token.line << ":" << token.position << "\n";

    if (token.type == TokenType::Identifer ||
        token.type == TokenType::StringLiteral ||
        token.type == TokenType::Error) {
      std::cout << "  " << token.as_string << " "
                << token.line << ":" << token.position << "\n";
    } else if (token.type == TokenType::NumberLiteral) {
      std::cout << "  " << token.as_number << " "
                << token.line << ":" << token.position << "\n";
    }

    token = lexer.next();
  }

  return EX_OK;
}
