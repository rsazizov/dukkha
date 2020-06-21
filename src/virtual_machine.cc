#include "virtual_machine.hh"

#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <ios>
#include <iostream>

Value::Value(ValueType type) {
  m_type = type;
}

Value::Value(double value) {
  m_type = ValueType::Number;
  m_number = value;
}

Value::Value(bool value) {
  m_type = ValueType::Bool;
  m_bool = value;
}

bool Value::is(ValueType type) const {
  return m_type == type;
}

ValueType Value::getType() const {
  return m_type;
}

double Value::as_number() const {
  // TODO: assert?
  return m_number;
}

bool Value::as_bool() const {
  // TODO: assert?
  return m_bool;
}

std::ostream& operator <<(std::ostream& os, const Value& value) {
  switch (value.getType()) {
    case ValueType::Number: os << value.as_number(); break;
    case ValueType::Bool: os << std::boolalpha << value.as_bool(); break;
    default: os << "Error"; break;
  }

  return os;
}

void Bytecode::clear() {
  m_code.clear();
  m_consts.clear();
  m_lines.clear();
}

std::size_t Bytecode::push_op(std::uint8_t op, std::size_t line) {
  m_code.push_back(op);
  m_lines.push_back(line);

  return m_code.size() - 1;
}

std::size_t Bytecode::push_const(Value value) {
  m_consts.push_back(value);
  return m_consts.size() - 1;
}

Value Bytecode::get_const(std::size_t address) const {
  return m_consts[address];
}

const std::vector<std::uint8_t>& Bytecode::get_code() const {
  return m_code;
}

void Bytecode::disassemble() {
  std::cout << "--- Bytecode dump ---\n";

  for (std::size_t i = 0; i < m_code.size(); ++i) {
    std::uint8_t op = m_code[i];

    std::cout << std::setfill('0');
    std::cout << std::setw(5) << i << ":" << std::setw(3) << m_lines[i] << " ";

    switch (op) {
      case VirtualMachine::Add: std::cout << "add\n"; break;
      case VirtualMachine::Subtract: std::cout << "sub\n"; break;
      case VirtualMachine::Multiply: std::cout << "mul\n"; break;
      case VirtualMachine::Divide: std::cout << "div\n"; break;
      case VirtualMachine::Negate: std::cout << "neg\n"; break;
      case VirtualMachine::Exp: std::cout << "exp\n"; break;
      case VirtualMachine::Constant16:
        std::cout << "push $" << (int) m_code[++i] << "\n";
        break;
      case VirtualMachine::Return: std::cout << "ret\n"; break;
    }
  }

  std::cout << "--------------------\n";
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
  switch (a.getType()) {
    case ValueType::Number:
      return -a.as_number();
      break;
    default:
      error() << "Unexpected operand type for negation";
      return Value(ValueType::Error);
  }
}

Value VirtualMachine::add(const Value& a, const Value& b) {
  switch (a.getType()) {
    case ValueType::Number:
      return a.as_number() + b.as_number();
      break;
    default:
      error() << "Unexpected operand type for sub";
      return Value(ValueType::Error);
  }
}

Value VirtualMachine::sub(const Value& a, const Value& b) {
  switch (a.getType()) {
    case ValueType::Number:
      return a.as_number() - b.as_number();
      break;
    default:
      error() << "Unexpected operand type for sub";
      return Value(ValueType::Error);
  }
}

Value VirtualMachine::mul(const Value& a, const Value& b) {
  switch (a.getType()) {
    case ValueType::Number:
      return a.as_number() * b.as_number();
      break;
    default:
      error() << "Unexpected operand type for mul";
      return Value(ValueType::Error);
  }
}

Value VirtualMachine::div(const Value& a, const Value& b) {
  switch (a.getType()) {
    case ValueType::Number:
      return a.as_number() / b.as_number();
      break;
    default:
      error() << "Unexpected operand type for div";
      return Value(ValueType::Error);
  }
}

Value VirtualMachine::exp(const Value& a, const Value& b) {
  switch (a.getType()) {
    case ValueType::Number:
      return std::pow(a.as_number(), b.as_number());
      break;
    default:
      error() << "Unexpected operand type for exp";
      return Value(ValueType::Error);
  }
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
        return pop();
      case Constant16: {
        Value value = read_const();
        m_stack.push_back(value);
        break;
      }
      case Negate:
        push(neg(pop())); break;
      case Add: {
        Value a = pop();
        Value b = pop();
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
  std::cout << "Runtime error on line " << m_code->m_lines[offset] << ": ";

  m_halt = true;

  return std::cout;
}
