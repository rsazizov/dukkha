#include "compiler.hh"
#include "lexer.hh"
#include "virtual_machine.hh"

#include <cstdint>
#include <iostream>
#include <math.h>
#include <string>

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
  m_block_depth = 0;

  while (m_cursor.type != TokenType::EndOfFile) {
    declaration();
  }

  emit_byte(VirtualMachine::Return);

  return !m_had_error;
}

void Compiler::declaration() {
  if (m_cursor.type == TokenType::Let) {
    advance();
    variable_declaration();
  } else if (m_cursor.type == TokenType::LeftCurly) {
    advance();
    block();
  } else {
    statement();
  }
}

void Compiler::block() {
  enter_block();

  while (m_cursor.type != TokenType::RightCurly &&
         m_cursor.type != TokenType::EndOfFile)  {
    declaration();
  }

  consume(TokenType::RightCurly, "'}' expected");
  leave_block();
}

void Compiler::variable_declaration() {
  consume(TokenType::Identifer, "variable name expeceted");

  bool global_scope = m_block_depth == 0;

  Value name = Value(m_prev.as_string);
  std::size_t pname = 0;

  if (global_scope) {
    pname = resolve_string(name.as_string());
    emit_byte(VirtualMachine::AllocGlobal);
    emit_byte(pname);
  } else {
    for (const auto& local : m_locals) {
      if (local.depth == m_block_depth && local.name == name.as_string()) {
        error(m_prev, "Redefinition of a local variable");
      }
    }
  }

  if (m_cursor.type == TokenType::Eq) {
    advance();
    expression();
  } else {
    emit_byte(VirtualMachine::LoadNull);
  }

  if (global_scope) {
    emit_byte(VirtualMachine::StoreGlobal);
    emit_byte(pname);
  } else {
    emit_byte(VirtualMachine::StoreLocal);
    emit_byte(m_locals.size());

    m_locals.push_back((LocalVar) {
        .depth = m_block_depth,
        .stack_offset = m_locals.size(),
        .name = name.as_string()
        });
  }

  consume(TokenType::Semicolon, "';' expeceted");
}

void Compiler::expression() {
  logical_or();
}

void Compiler::statement() {
  if (m_cursor.type == TokenType::Print) {
    advance();
    print();
  } else if (m_cursor.type == TokenType::Identifer) {
    advance();
    variable_assignment();
  } else if (m_cursor.type == TokenType::If) {
    advance();
    if_statement();
  } else if (m_cursor.type == TokenType::While) {
    advance();
    while_statement();
  } else {
    expression();
    consume(TokenType::Semicolon, "';' expected");
  }
}

void Compiler::variable_assignment() {
  const std::string& name = m_prev.as_string;
  std::size_t pname = resolve_string(name);

  consume(TokenType::Eq, "'=' expected");

  expression();

  consume(TokenType::Semicolon, "';' expected");

  emit_byte(VirtualMachine::StoreGlobal);
  emit_byte(pname);
}

void Compiler::print() {
  expression();
  emit_byte(VirtualMachine::Print);
  consume(TokenType::Semicolon, "';' expected");
}

void Compiler::if_statement() {
  expression();
  consume(TokenType::LeftCurly, "'{' expected");

  std::vector<std::size_t> endif_jumps;
  std::size_t next_block_target = 0;

  emit_byte(VirtualMachine::JumpIfFalse);
  next_block_target = emit_qword(0);

  block();

  emit_byte(VirtualMachine::Jump);
  endif_jumps.push_back(emit_qword(0));

  m_code.set_qword(next_block_target, m_code.get_code().size());

  while (m_cursor.type == TokenType::Else) {
    advance();

    if (m_cursor.type == TokenType::If) {
      advance();

      expression();

      emit_byte(VirtualMachine::JumpIfFalse);
      next_block_target = emit_qword(0);

      consume(TokenType::LeftCurly, "'{' expected");
      block();

      emit_byte(VirtualMachine::Jump);
      endif_jumps.push_back(emit_qword(0));

      m_code.set_qword(next_block_target, m_code.get_code().size());
    } else {
      consume(TokenType::LeftCurly, "'{' expected");
      block();
      break;
    }
  }

  for (auto address : endif_jumps) {
    m_code.set_qword(address, m_code.get_code().size());
  }
}

void Compiler::while_statement() {
  std::size_t eval_exp_address = m_code.get_code().size();
  expression();

  emit_byte(VirtualMachine::JumpIfFalse);
  std::size_t block_end_addr = emit_qword(0);

  consume(TokenType::LeftCurly, "'{' expected");

  block();

  emit_byte(VirtualMachine::Jump);
  emit_qword(eval_exp_address);

  m_code.set_qword(block_end_addr, m_code.get_code().size());

  if (m_cursor.type == TokenType::Else) {
    advance();
    consume(TokenType::LeftCurly, "'{' expected");
    block();
  }
}

void Compiler::logical_or() {
  logical_and();

  while (m_cursor.type == TokenType::Or) {
    advance();
    logical_and();
    emit_byte(VirtualMachine::Or);
  }
}

void Compiler::logical_and() {
  logical_not();

  while (m_cursor.type == TokenType::And) {
    advance();
    logical_not();
    emit_byte(VirtualMachine::And);
  }
}

void Compiler::logical_not() {
  if (m_cursor.type == TokenType::Not) {
    comparison();
    emit_byte(VirtualMachine::Not);
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
        emit_byte(VirtualMachine::Equal);
        break;
      case TokenType::BangEq:
        emit_byte(VirtualMachine::Equal);
        emit_byte(VirtualMachine::Not);
        break;
      case TokenType::GreaterEq:
        emit_byte(VirtualMachine::Less);
        emit_byte(VirtualMachine::Not);
        break;
      case TokenType::LessEq:
        emit_byte(VirtualMachine::Greater);
        emit_byte(VirtualMachine::Not);
        break;
      case TokenType::Greater:
        emit_byte(VirtualMachine::Greater);
        break;
      case TokenType::Less:
        emit_byte(VirtualMachine::Less);
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

    emit_byte(op);
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
    emit_byte(op);
  }
}

void Compiler::exp() {
  arbitrary();

  while (m_cursor.type == TokenType::StarStar) {
    advance();
    arbitrary();
    emit_byte(VirtualMachine::Exp);
  }
}

void Compiler::unary() {
  if (m_cursor.type == TokenType::Minus) {
    advance();
    exp();
    emit_byte(VirtualMachine::Negate);
  } else {
    exp();
  }
}

void Compiler::arbitrary() {
  switch (m_cursor.type) {
    case TokenType::NumberLiteral: {
      std::size_t pa = m_code.push_const(m_cursor.as_number);
      emit_byte(VirtualMachine::Constant16);
      emit_byte(pa);
      advance();
      break;
    }
    case TokenType::LeftRound:
      advance();
      expression();
      consume(TokenType::RightRound, "')' expected");
      break;
    case TokenType::StringLiteral: {
      std::size_t pa = resolve_string(m_cursor.as_string);
      emit_byte(VirtualMachine::Constant16);
      emit_byte(pa);
      advance();
      break;
    }
    case TokenType::True: {
      std::size_t pa = m_code.push_const(true);
      emit_byte(VirtualMachine::Constant16);
      emit_byte(pa);
      advance();
      break;
    }
    case TokenType::False: {
      std::size_t pa = m_code.push_const(false);
      emit_byte(VirtualMachine::Constant16);
      emit_byte(pa);
      advance();
      break;
    }
    case TokenType::Identifer: {
      resolve_variable(m_cursor.as_string);
      advance();
      break;
    }
    default:
      error(m_cursor, "invalid syntax");
  }
}

void Compiler::enter_block() {
  m_block_depth++;
}

void Compiler::leave_block() {
  while (!m_locals.empty() && m_locals.back().depth == m_block_depth) {
    m_locals.pop_back();
    emit_byte(VirtualMachine::Pop);
  }

  m_block_depth--;
}

void Compiler::resolve_variable(const std::string& name) {
  for (auto local = m_locals.rbegin(); local != m_locals.rend(); ++local) {
    if (local->name == name && local->depth <= m_block_depth) {
      emit_byte(VirtualMachine::LoadLocal);
      emit_byte(local->stack_offset);

      return;
    }
  }

  std::size_t pname = resolve_string(name);
  emit_byte(VirtualMachine::LoadGlobal);
  emit_byte(pname);
}


std::size_t Compiler::resolve_string(const std::string& name) {
  auto it = m_strings.find(name);

  if (it == m_strings.end()) {
    std::size_t address = m_code.push_const(name);
    m_strings[name] = address;
    return address;
  }

  return it->second;
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
    advance();
  }
}

std::size_t Compiler::emit_byte(std::uint8_t byte) {
  return m_code.push_byte(byte, m_prev.line);
}

std::size_t Compiler::emit_qword(std::size_t qword) {
  return m_code.push_qword(qword, m_prev.line);
}

void Compiler::error(const Token& at, const char* msg) {
  m_had_error = true;
  std::cout << "Error at: " << at.line << ":"
            << at.position << " - " << msg << "\n";
}
