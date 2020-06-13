#include "compiler.hh"
#include "lexer.hh"
#include "virtual_machine.hh"

#include <cstdint>
#include <iostream>

Compiler::Compiler() {
}

bool Compiler::from_file(const char* path, Bytecode& bytecode) {
  m_lexer.from_file(path);
  m_peek = m_lexer.next();

  bool result = compile();
  bytecode = m_code;

  return result;
}

bool Compiler::compile() {
  advance();

  expression();
  emit(VirtualMachine::Return);

  return true;
}

bool is_operator(TokenType type) {
  return type == TokenType::Plus ||
    type == TokenType::Minus ||
    type == TokenType::Star ||
    type == TokenType::Slash;
}

void Compiler::expression() {
  switch (m_cursor.type) {
    case TokenType::Minus:
      advance();
      expression();
      emit(VirtualMachine::Negate);
      break;
    case TokenType::NumberLiteral: {
      Value a = m_cursor.as_number;
      std::uint8_t pa = m_code.push_const(a);
      emit(VirtualMachine::Constant16);
      emit(pa);

      if (m_peek.type == TokenType::Plus) {
        consume(TokenType::Plus, "Expected operator");

        Value b = m_peek.as_number;
        std::size_t pb = m_code.push_const(b);
        emit(VirtualMachine::Constant16);
        emit(pb);

        emit(VirtualMachine::Add);
      }

      break;
    }

    case TokenType::LeftRound:
      advance();
      expression();
      /* consume(TokenType::RightRound, "expected ')'"); */
      break;

    default:
      break;
  }
}

void Compiler::advance() {
  // Skip any error tokens.
  /* while (m_cursor.type == TokenType::Error) { */
  /*   error(m_cursor, ""); */
  /*   m_peek = m_lexer.next(); */
  /* } */

  m_cursor = m_peek;
  m_peek = m_lexer.next();
}

void Compiler::consume(TokenType type, const char* msg) {
  if (m_peek.type == type) {
    advance();
  } else {
    error(m_peek, msg);
  }
}

void Compiler::emit(std::uint8_t byte) {
  m_code.push_op(byte, m_cursor.line);
}

void Compiler::error(const Token at, const char* msg) {
  std::cout << "Error at: " << at.line << ":"
            << at.position << " - " << msg << "\n";
}
