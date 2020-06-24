#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "lexer.hh"
#include "virtual_machine.hh"

class Bytecode;

struct LocalVar {
  std::size_t depth { 0 };
  std::size_t stack_offset { 0 };
  std::string name {};

  bool operator ==(const LocalVar& b) const {
    return depth == b.depth &&
           stack_offset == b.stack_offset &&
           name == b.name;
  }
};

namespace std {

  template<>
  struct hash<LocalVar> {
    std::size_t operator()(const LocalVar& lv) {
      return std::hash<std::string>()(lv.name) ^
             std::hash<std::size_t>()(lv.stack_offset) ^
             std::hash<std::size_t>()(lv.depth);
    }
  };

}

class Compiler {
public:
  Compiler();
  ~Compiler() = default;

  bool from_file(const char* path, Bytecode& bytecode);

private:
  bool compile();

  void declaration();
  void block();
  void variable_declaration();

  void statement();
  void variable_assignment();

  void print();
  void if_statement();
  void while_statement();

  void expression();

  void logical_or();
  void logical_and();
  void logical_not();
  void comparison();

  void addition();
  void multiplication();
  void exp();
  void unary();
  void arbitrary();

  void enter_block();
  void leave_block();
  void resolve_variable(const std::string& name);

  void error(const Token& at, const char* msg);

  void advance();
  void consume(TokenType type, const char* msg);

  std::size_t emit(std::uint8_t byte);

  Bytecode m_code {};

  Token m_prev {};
  Token m_cursor {};

  Lexer m_lexer;

  std::size_t m_block_depth { 0 };

  std::vector<LocalVar> m_locals;
  // (depth, name) -> stack offset

  bool m_had_error { false };
};
