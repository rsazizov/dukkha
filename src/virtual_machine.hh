#pragma once

#include <cstdint>
#include <vector>
#include <ostream>

enum class ValueType : std::uint8_t {
  // TODO: float and int
  Number,
  Bool,
  Symbol,
  String,

  // Used internally.
  Error
};

class Value {
public:
  Value(ValueType type);
  Value(double value);
  Value(bool value);

  bool is(ValueType type) const;

  ValueType getType() const;

  double as_number() const;
  bool as_bool() const;
private:
  ValueType m_type;

  union {
    double m_number { 0.0 };
    bool m_bool;
  };
};

std::ostream& operator <<(std::ostream& os, const Value& value);

class Bytecode {
public:
  Bytecode() = default;
  ~Bytecode() = default;

  void clear();

  std::size_t push_op(std::uint8_t op, std::size_t line);
  std::size_t push_const(Value value);
  Value get_const(std::size_t address) const;

  const std::vector<std::uint8_t>& get_code() const;

  void disassemble();
private:
  friend class VirtualMachine;

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
    Exp,
    Divide
  };

  VirtualMachine();
  ~VirtualMachine();

  Value neg(const Value& a);
  Value add(const Value& a, const Value& b);
  Value sub(const Value& a, const Value& b);
  Value mul(const Value& a, const Value& b);
  Value div(const Value& a, const Value& b);
  Value exp(const Value& a, const Value& b);

  Value execute(const Bytecode* code);

  void halt();
  std::ostream& error();

  void push(Value value);
  Value pop();
private:
  void error(const char* msg);

  bool m_halt = false;

  const Bytecode* m_code { nullptr };
  const std::uint8_t* m_ip { nullptr };
  std::vector<Value> m_stack;
};
