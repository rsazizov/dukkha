#include "compiler.hh"
#include "lexer.hh"
#include "virtual_machine.hh"

#include <cstdint>
#include <iostream>

Compiler::Compiler() {
}

bool Compiler::from_file(const char* path, Bytecode& bytecode) {
  m_lexer.from_file(path);
  m_cursor = m_lexer.next();

  bool result = compile();

  if (result) {
    bytecode = m_code;
    return true;
  } else {
    return false;
  }
}

bool Compiler::compile() {
  m_had_error = false;

  expression();

  consume(TokenType::EndOfFile, "EOF expected");
  emit(VirtualMachine::Return);

  return !m_had_error;
}

bool is_operator(TokenType type) {
  return type == TokenType::Plus ||
    type == TokenType::Minus ||
    type == TokenType::Star ||
    type == TokenType::Slash;
}

void Compiler::expression() {
  addition();
}

void Compiler::addition() {
  // TODO: assert?
  multiplication();

  VirtualMachine::Instruction op {};

  while (m_cursor.type == TokenType::Plus || m_cursor.type == TokenType::Minus) {
    switch (m_cursor.type) {
      case TokenType::Plus: op = VirtualMachine::Add; break;
      case TokenType::Minus: op = VirtualMachine::Subtract; break;
      default: error(m_cursor, "Unexpected token.");
    }

    advance();
    multiplication();

    emit(op);
  }
}

void Compiler::multiplication() {
  exp();

  VirtualMachine::Instruction op {};

  while (m_cursor.type == TokenType::Slash || m_cursor.type == TokenType::Star) {
    switch (m_cursor.type) {
      case TokenType::Star: op = VirtualMachine::Multiply; break;
      case TokenType::Slash: op = VirtualMachine::Divide; break;
      default: error(m_cursor, "Unexpected token.");
    }

    advance();
    exp();
    emit(op);
  }
}

void Compiler::exp() {
  unary();

  while (m_cursor.type == TokenType::StarStar) {
    advance();
    unary();
    emit(VirtualMachine::Exp);
  }
}

void Compiler::unary() {
  if (m_cursor.type == TokenType::Minus) {
    advance();
    unary();
    emit(VirtualMachine::Negate);
  } else {
    arbitrary();
  }
}

void Compiler::arbitrary() {
  switch (m_cursor.type) {
    case TokenType::NumberLiteral: {
      std::size_t pa = m_code.push_const(m_cursor.as_number);
      emit(VirtualMachine::Constant16);
      emit(pa);
      advance();
      break;
    }
    case TokenType::LeftRound:
      advance();
      expression();
      consume(TokenType::RightRound, "')' expected");
      break;
    default:
      error(m_cursor, "unexpected token.");
  }
}

void Compiler::advance() {
  m_prev = m_cursor;
  m_cursor = m_lexer.next();
}

void Compiler::consume(TokenType type, const char* msg) {
  if (m_cursor.type == type) {
    advance();
  } else {
    error(m_cursor, msg);
  }
}

void Compiler::emit(std::uint8_t byte) {
  if (!m_had_error) {
    m_code.push_op(byte, m_prev.line);
  }
}

void Compiler::error(const Token& at, const char* msg) {
  m_had_error = true;
  std::cout << "Error at: " << at.line << ":"
            << at.position << " - " << msg
            << ", got " << at.type << ".\n";
}
