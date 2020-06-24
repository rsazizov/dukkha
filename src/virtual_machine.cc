#include "virtual_machine.hh"
#include "value.hh"

#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <ios>
#include <math.h>
#include <sstream>
#include <iostream>

void Bytecode::clear() {
  m_code.clear();
  m_consts.clear();
  m_lines.clear();
}

// TODO: Sanity checks.

std::size_t Bytecode::push_op(std::uint8_t op, std::size_t line) {
  m_code.push_back(op);
  m_lines.push_back(line);

  return m_code.size() - 1;
}

std::size_t Bytecode::push_const(Value value) {
  m_consts.push_back(value);
  return m_consts.size() - 1;
}

void Bytecode::set_op(std::size_t address, std::uint8_t op) {
  m_code[address] = op;
}

std::uint8_t& Bytecode::get_op(std::size_t address) {
  return m_code[address];
}

Value Bytecode::get_const(std::size_t address) const {
  return m_consts[address];
}

const std::vector<std::uint8_t>& Bytecode::get_code() const {
  return m_code;
}

void Bytecode::dump_data() {
  std::cout << ".rodata:\n";
  for (std::size_t i = 0; i < m_consts.size(); ++i) {
    std::cout << std::setfill('0');
    std::cout << "$" << std::setw(5) << i << " " << m_consts[i] << "\n";
  }
}

void Bytecode::dump_text() {
  std::cout << ".text:\n";

  for (std::size_t i = 0; i < m_code.size(); ++i) {
    std::uint8_t op = m_code[i];

    std::cout << std::setfill('0');
    std::cout << "$" << std::setw(5) << i << ":" << std::setw(3) << m_lines[i] << " ";

    switch (op) {
      case VirtualMachine::Add: std::cout << "add\n"; break;
      case VirtualMachine::Subtract: std::cout << "sub\n"; break;
      case VirtualMachine::Multiply: std::cout << "mul\n"; break;
      case VirtualMachine::Divide: std::cout << "div\n"; break;
      case VirtualMachine::Negate: std::cout << "neg\n"; break;
      case VirtualMachine::Not: std::cout << "not\n"; break;
      case VirtualMachine::And: std::cout << "and\n"; break;
      case VirtualMachine::Or: std::cout << "or\n"; break;
      case VirtualMachine::Equal: std::cout << "eq\n"; break;
      case VirtualMachine::Greater: std::cout << "gt\n"; break;
      case VirtualMachine::Less: std::cout << "lt\n"; break;
      case VirtualMachine::Exp: std::cout << "exp\n"; break;
      case VirtualMachine::LoadNull: std::cout << "lnull\n"; break;
      case VirtualMachine::Print: std::cout << "cout\n"; break;
      case VirtualMachine::Pop: std::cout << "pop\n"; break;
      case VirtualMachine::Return: std::cout << "ret\n"; break;

      case VirtualMachine::AllocGlobal:
        std::cout << "alcg $" << (std::size_t) m_code[++i] << "\n";
        break;
      case VirtualMachine::StoreGlobal:
        std::cout << "stg $" << (std::size_t) m_code[++i] << "\n";
        break;
      case VirtualMachine::LoadGlobal:
        std::cout << "loadg $" << (std::size_t) m_code[++i] << "\n";
        break;
      case VirtualMachine::StoreLocal:
        std::cout << "stl %" << (std::size_t) m_code[++i] << "\n";
        break;
      case VirtualMachine::LoadLocal:
        std::cout << "loadl %" << (std::size_t) m_code[++i] << "\n";
        break;
      case VirtualMachine::Jump:
        std::cout << "jmp $" << (std::size_t) m_code[++i] << "\n";
        break;
      case VirtualMachine::JumpIfFalse:
        std::cout << "jmpf $" << (std::size_t) m_code[++i] << "\n";
        break;
      case VirtualMachine::Constant16:
        std::cout << "push $" << (std::size_t) m_code[++i] << "\n";
        break;
    }
  }
}

VirtualMachine::VirtualMachine() {
  m_stack.reserve(256);
}

VirtualMachine::~VirtualMachine() {
}

void VirtualMachine::push(Value value) {
  m_stack.push_back(value);
}

Value VirtualMachine::pop() {
  Value value = m_stack.back();
  m_stack.pop_back();

  return value;
}

Value VirtualMachine::neg(const Value& a) {
  if (a.is(ValueType::Number)) {
    return -a.as_number();
  }

  error() << "Unexpected operand type: -" << a.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::add(const Value& a, const Value& b) {
  if (a.getType() == b.getType()) {
    if (a.getType() == ValueType::Number) {
      return a.as_number() + b.as_number();
    } else if (a.getType() == ValueType::String) {
      return a.as_string() + b.as_string();
    }
  }

  error() << "Unexpected operand types: " << a.getType()
          << "+" << b.getType() << "\n";

  return Value(ValueType::Error);
}

Value VirtualMachine::sub(const Value& a, const Value& b) {
  if (a.is(ValueType::Number) && b.is(ValueType::Number)) {
    return a.as_number() - b.as_number();
  }

  error() << "Unexpected operand types: " << a.getType()
          << "-" << b.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::mul(const Value& a, const Value& b) {
  if (a.is(ValueType::Number) && b.is(ValueType::Number)) {
      return a.as_number() * b.as_number();
  } else if (a.is(ValueType::String) && b.is(ValueType::Number)) {
    std::stringstream ss("");

    for (std::size_t i = 0; i < b.as_number(); ++i) {
      ss << a.as_string();
    }

    return Value(ss.str());
  } else if (a.is(ValueType::Number) && b.is(ValueType::String)) {
    return mul(b, a);
  }

  error() << "Unexpected operand types: " << a.getType()
          << "*" << b.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::div(const Value& a, const Value& b) {
  if (a.is(ValueType::Number) && b.is(ValueType::Number)) {
    return a.as_number() / b.as_number();
  }

  error() << "Unexpected operand types: " << a.getType()
          << "/" << b.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::exp(const Value& a, const Value& b) {
  if (a.is(ValueType::Number) && b.is(ValueType::Number)) {
    return std::pow(a.as_number(), b.as_number());
  }

  error() << "Unexpected operand types: " << a.getType()
          << "**" << b.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::logical_not(const Value& a) {
  if (a.is(ValueType::Bool)) {
    return !a.as_bool();
  }

  error() << "Unexpected operand type: not " << a.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::logical_and(const Value& a, const Value& b) {
  if (a.is(ValueType::Bool) && b.is(ValueType::Bool)) {
    return a.as_bool() && b.as_bool();
  }

  error() << "Unexpected operand type: " << a.getType()
          << " and " << b.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::logical_or(const Value& a, const Value& b) {
  if (a.is(ValueType::Bool) && b.is(ValueType::Bool)) {
    return a.as_bool() || b.as_bool();
  }

  error() << "Unexpected operand type: " << a.getType()
          << " or " << b.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::logical_equals(const Value& a, const Value& b) {
  if (a.is(ValueType::Bool) && b.is(ValueType::Bool)) {
    return a.as_bool() == b.as_bool();
  } else if (a.is(ValueType::Number) && b.is(ValueType::Number)) {
    return a.as_number() == b.as_number();
  }

  error() << "Unexpected operand type: " << a.getType()
          << " == " << b.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::logical_greater(const Value& a, const Value& b) {
  if (a.is(ValueType::Number) && b.is(ValueType::Number)) {
    return a.as_number() > b.as_number();
  }

  error() << "Unexpected operand type: " << a.getType()
          << " > " << b.getType() << "\n";
  return Value(ValueType::Error);
}

Value VirtualMachine::logical_less(const Value& a, const Value& b) {
  if (a.is(ValueType::Number) && b.is(ValueType::Number)) {
    return a.as_number() < b.as_number();
  }

  error() << "Unexpected operand type: " << a.getType()
          << " > " << b.getType() << "\n";
  return Value(ValueType::Error);
}


void VirtualMachine::alloc_global(const Value& name) {
  auto global = m_globals.find(name.as_string());

  if (global != m_globals.end()) {
    error() << "Name '" << name << "' has already been defined" << ".\n";
    return;
  }

  m_globals[name.as_string()] = Value();
}

void VirtualMachine::store_global(const Value& name, const Value& value) {
  if (!name.is(ValueType::String)) {
    error() << "Unexpected global name type: " << name.getType() << "\n";
    return;
  }

  auto global = m_globals.find(name.as_string());

  if (global == m_globals.end()) {
    error() << "Name '" << name << "' is not known" << ".\n";
    return;
  }

  global->second = value;
}


void VirtualMachine::load_global(const Value& name) {
  if (!name.is(ValueType::String)) {
    error() << "Unexpected global name type: " << name.getType() << ".\n";
    return;
  }

  auto global = m_globals.find(name.as_string());

  if (global == m_globals.end()) {
    error() << "Name '" << name << "' is not known" << ".\n";
    return;
  }

  push(global->second);
}

Value VirtualMachine::execute(const Bytecode* code) {
  m_code = code;
  m_ip = code->get_code().data();

  auto read_const = [&]() {
    return code->get_const(*m_ip++);
  };

  while (m_ip != nullptr && !m_halt) {
    std::uint8_t op = *m_ip++;

    switch (op) {
      case Return:
        return true;
      case Constant16: {
        Value value = read_const();
        push(value);
        break;
      }
      case Pop: {
        pop();
        break;
      }
      case Negate:
        push(neg(pop())); break;
      case Add: {
        Value b = pop();
        Value a = pop();
        push(add(a, b));

        break;
      }
      case Subtract: {
        Value b = pop();
        Value a = pop();
        push(sub(a, b));

        break;
      }
      case Divide: {
        Value b = pop();
        Value a = pop();
        push(div(a, b));

        break;
      }
      case Multiply: {
        Value b = pop();
        Value a = pop();
        push(mul(a, b));

        break;
      }
      case Exp: {
        Value b = pop();
        Value a = pop();
        push(exp(a, b));

        break;
      }
      case Not: {
        Value a = pop();
        push(logical_not(a));
        break;
      }
      case And: {
        Value b = pop();
        Value a = pop();
        push(logical_and(a, b));
        break;
      }
      case Or: {
        Value b = pop();
        Value a = pop();
        push(logical_or(a, b));
        break;
      }
      case Equal: {
        Value b = pop();
        Value a = pop();
        push(logical_equals(a, b));
        break;
      }
      case Greater: {
        Value b = pop();
        Value a = pop();
        push(logical_greater(a, b));
        break;
      }
      case Less: {
        Value b = pop();
        Value a = pop();
        push(logical_less(a, b));
        break;
      }
      case Print: {
        Value a = pop();
        std::cout << a << "\n";
        break;
      }
      case LoadNull: {
        push(Value());
        break;
      }
      case AllocGlobal: {
        Value name = read_const();
        alloc_global(name);
        break;
      }
      case StoreGlobal: {
        Value value = pop();
        Value name = read_const();

        store_global(name, value);
        break;
      }
      case LoadGlobal: {
        Value name = read_const();
        load_global(name);
        break;
      }
      case StoreLocal: {
        auto stack_offset = *m_ip++;
        m_stack[stack_offset] = m_stack.back();
        break;
      }
      case LoadLocal: {
        auto stack_offset = *m_ip++;
        push(m_stack[stack_offset]);
        break;
      }
      case Jump: {
        auto offset = *m_ip++;
        m_ip = code->get_code().data() + offset;
        break;
      }
      case JumpIfFalse: {
        auto offset = *m_ip++;

        if (!pop().as_bool()) {
          m_ip = code->get_code().data() + offset;
        }

        break;
      }
      default: error() << "Unexpected op: " << (std::size_t) op << "\n";
    }
  }

  halt();
  return Value(ValueType::Error);
}

void VirtualMachine::halt() {
  m_stack.clear();
  m_ip = nullptr;
  m_halt = true;
  m_code = nullptr;
}

std::ostream& VirtualMachine::error() {
  std::size_t offset = (std::size_t) (m_ip - m_code->m_code.data());
  std::cout << "Runtime error on " << m_code->m_lines[offset] << ":" << offset << ": ";

  m_halt = true;

  return std::cout;
}
