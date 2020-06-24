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

  emit(VirtualMachine::Return);

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
    emit(VirtualMachine::AllocGlobal);
    emit(pname);
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
    emit(VirtualMachine::LoadNull);
  }

  if (global_scope) {
    emit(VirtualMachine::StoreGlobal);
    emit(pname);
  } else {
    emit(VirtualMachine::StoreLocal);
    emit(m_locals.size());

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

  emit(VirtualMachine::StoreGlobal);
  emit(pname);
}

void Compiler::print() {
  expression();
  emit(VirtualMachine::Print);
  consume(TokenType::Semicolon, "';' expected");
}

void Compiler::if_statement() {
  expression();
  consume(TokenType::LeftCurly, "'{' expected");

  std::vector<std::size_t> endif_jumps;
  std::size_t next_block_target = 0;

  emit(VirtualMachine::JumpIfFalse);
  next_block_target = emit(0);

  block();

  emit(VirtualMachine::Jump);
  endif_jumps.push_back(emit(0));

  m_code.set_op(next_block_target, m_code.get_code().size());

  while (m_cursor.type == TokenType::Else) {
    advance();

    if (m_cursor.type == TokenType::If) {
      advance();

      expression();

      emit(VirtualMachine::JumpIfFalse);
      next_block_target = emit(0);

      consume(TokenType::LeftCurly, "'{' expected");
      block();

      emit(VirtualMachine::Jump);
      endif_jumps.push_back(emit(0));

      m_code.set_op(next_block_target, m_code.get_code().size());
    } else {
      consume(TokenType::LeftCurly, "'{' expected");
      block();
      break;
    }
  }

  for (auto address : endif_jumps) {
    m_code.set_op(address, m_code.get_code().size());
  }
}

void Compiler::while_statement() {
  std::size_t eval_exp_address = m_code.get_code().size();
  expression();

  emit(VirtualMachine::JumpIfFalse);
  std::size_t block_end_addr = emit(0);

  consume(TokenType::LeftCurly, "'{' expected");

  block();

  emit(VirtualMachine::Jump);
  emit(eval_exp_address);

  m_code.set_op(block_end_addr, m_code.get_code().size());

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
      std::size_t pa = resolve_string(m_cursor.as_string);
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
    emit(VirtualMachine::Pop);
  }

  m_block_depth--;
}

void Compiler::resolve_variable(const std::string& name) {
  for (auto local = m_locals.rbegin(); local != m_locals.rend(); ++local) {
    if (local->name == name && local->depth <= m_block_depth) {
      emit(VirtualMachine::LoadLocal);
      emit(local->stack_offset);

      return;
    }
  }

  std::size_t pname = resolve_string(name);
  emit(VirtualMachine::LoadGlobal);
  emit(pname);
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

std::size_t Compiler::emit(std::uint8_t byte) {
  return m_code.push_op(byte, m_prev.line);
}

void Compiler::error(const Token& at, const char* msg) {
  m_had_error = true;
  std::cout << "Error at: " << at.line << ":"
            << at.position << " - " << msg << "\n";
}
