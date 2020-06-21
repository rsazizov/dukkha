#include "virtual_machine.hh"

#include <iomanip>
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

Value VirtualMachine::execute(const Bytecode* code) {
  m_ip = code->get_code().data();

  auto read_const = [&]() {
    return code->get_const(*m_ip++);
  };

  while (m_ip != nullptr) {
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
      case Add: {
        Value a = pop();
        Value b = pop();
        push(a + b);

        break;
      }
      case Subtract: {
        Value b = pop();
        Value a = pop();
        push(a - b);

        break;
      }
      case Divide: {
        Value b = pop();
        Value a = pop();
        push(a / b);

        break;
      }
      case Multiply: {
        Value b = pop();
        Value a = pop();
        push(a * b);

        break;
      }
    }
  }

  return -1;
}
