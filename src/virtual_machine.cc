#include "virtual_machine.hh"
#include <iostream>

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

Value VirtualMachine::execute(const Bytecode* code) {
  m_ip = code->get_code().data();

  auto read_const = [&]() {
    return code->get_const(*m_ip++);
  };

  while (true) {
    std::uint8_t op = *m_ip++;

    switch (op) {
      case Return:
        return pop();
      case Constant16: {
        Value value = read_const();
        m_stack.push_back(value);
        break;
      }
      case Negate: push(-pop()); break;
    }
  }

  return -1;
}
