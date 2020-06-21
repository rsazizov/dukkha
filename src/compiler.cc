#include "compiler.hh"
#include "lexer.hh"
#include "virtual_machine.hh"

#include <cstdint>
#include <iostream>
#include <math.h>

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

  while (m_cursor.type != TokenType::EndOfFile) {
    statement();
  }

  emit(VirtualMachine::Return);

  return !m_had_error;
}

void Compiler::expression() {
  logical_or();
}

void Compiler::statement() {
  if (m_cursor.type == TokenType::Print) {
    advance();
    print();
  } else {
    expression();
    consume(TokenType::Semicolon, "';' expected");
  }
}

void Compiler::print() {
  expression();
  emit(VirtualMachine::Print);
  consume(TokenType::Semicolon, "';' expected");
}

void Compiler::logical_or() {
  logical_and();

  while (m_cursor.type == TokenType::Or) {
    advance();
    logical_and();
    emit(VirtualMachine::Or);
  }
}

void Compiler::logical_and() {
  logical_not();

  while (m_cursor.type == TokenType::And) {
    advance();
    logical_not();
    emit(VirtualMachine::And);
  }
}

void Compiler::logical_not() {
  if (m_cursor.type == TokenType::Not) {
    comparison();
    emit(VirtualMachine::Not);
  } else {
    comparison();
  }
}

bool is_comparison_op(TokenType type) {
  switch (type) {
    case TokenType::EqEq:
    case TokenType::BangEq:
    case TokenType::GreaterEq:
    case TokenType::LessEq:
    case TokenType::Greater:
    case TokenType::Less:
      return true;
    default:
      return false;
  }
}

void Compiler::comparison() {
  addition();

  while (is_comparison_op(m_cursor.type)) {
    TokenType op = m_cursor.type;

    advance();
    addition();

    switch (op) {
      case TokenType::EqEq:
        emit(VirtualMachine::Equal);
        break;
      case TokenType::BangEq:
        emit(VirtualMachine::Equal);
        emit(VirtualMachine::Not);
        break;
      case TokenType::GreaterEq:
        emit(VirtualMachine::Less);
        emit(VirtualMachine::Not);
        break;
      case TokenType::LessEq:
        emit(VirtualMachine::Greater);
        emit(VirtualMachine::Not);
        break;
      case TokenType::Greater:
        emit(VirtualMachine::Greater);
        break;
      case TokenType::Less:
        emit(VirtualMachine::Less);
        break;
      default: break;
    }
  }
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
  unary();

  VirtualMachine::Instruction op {};

  while (m_cursor.type == TokenType::Slash || m_cursor.type == TokenType::Star) {
    switch (m_cursor.type) {
      case TokenType::Star: op = VirtualMachine::Multiply; break;
      case TokenType::Slash: op = VirtualMachine::Divide; break;
      default: error(m_cursor, "Unexpected token.");
    }

    advance();
    unary();
    emit(op);
  }
}

void Compiler::exp() {
  arbitrary();

  while (m_cursor.type == TokenType::StarStar) {
    advance();
    arbitrary();
    emit(VirtualMachine::Exp);
  }
}

void Compiler::unary() {
  if (m_cursor.type == TokenType::Minus) {
    advance();
    exp();
    emit(VirtualMachine::Negate);
  } else {
    exp();
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
    case TokenType::StringLiteral: {
      std::size_t pa = m_code.push_const(m_cursor.as_string);
      emit(VirtualMachine::Constant16);
      emit(pa);
      advance();
      break;
    }
    case TokenType::True: {
      std::size_t pa = m_code.push_const(true);
      emit(VirtualMachine::Constant16);
      emit(pa);
      advance();
      break;
    }
    case TokenType::False: {
      std::size_t pa = m_code.push_const(false);
      emit(VirtualMachine::Constant16);
      emit(pa);
      advance();
      break;
    }
    default:
      error(m_cursor, "invalid syntax");
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
