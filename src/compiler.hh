#pragma once

#include <cstdint>

#include "lexer.hh"
#include "virtual_machine.hh"

class Bytecode;

class Compiler {
public:
  Compiler();
  ~Compiler() = default;

  bool from_file(const char* path, Bytecode& bytecode);

private:
  bool compile();

  void expression();
  void addition();
  void multiplication();
  void exp();
  void unary();
  void arbitrary();

  void error(const Token& at, const char* msg);

  void advance();
  void consume(TokenType type, const char* msg);

  void emit(std::uint8_t byte);

  Bytecode m_code {};

  Token m_prev {};
  Token m_cursor {};

  Lexer m_lexer;

  bool m_had_error { false };
};
