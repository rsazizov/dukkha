#pragma once

#include <cstdint>
#include <vector>

typedef double Value;

class Bytecode {
public:
  Bytecode() = default;
  ~Bytecode() = default;

  void clear();

  std::size_t push_op(std::uint8_t op, std::size_t line);
  std::size_t push_const(Value value);
  Value get_const(std::size_t address) const;

  const std::vector<std::uint8_t>& get_code() const;
private:
  std::vector<std::size_t> m_lines;
  std::vector<Value> m_consts;
  std::vector<std::uint8_t> m_code;
};

class VirtualMachine {
public:
  enum Instruction : std::uint8_t {
    Return,
    Constant16,
    Negate,
    Add,
    Subtract,
    Multiply,
    Divide
  };

  VirtualMachine();
  ~VirtualMachine();

  Value execute(const Bytecode* code);

  void push(Value value);
  Value pop();
private:
  /* const Bytecode* m_code { nullptr }; */
  const std::uint8_t* m_ip { nullptr };
  std::vector<Value> m_stack;
};
