#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <ostream>
#include <unordered_map>

#include "value.hh"

struct QwordToBytes {
  union {
    std::uint8_t bytes[8];
    std::size_t qword;
  };
};

class Bytecode {
public:
  Bytecode() = default;
  ~Bytecode() = default;

  void clear();

  std::size_t push_byte(std::uint8_t byte, std::size_t line);
  // TODO: size_t -> std::uint64_t?
  std::size_t push_qword(std::size_t qword, std::size_t line);
  std::size_t push_const(Value value);

  void set_byte(std::size_t address, std::uint8_t byte);
  void set_qword(std::size_t address, std::size_t qword);

  std::uint8_t get_byte(std::size_t address);
  std::size_t get_qword(std::size_t address);

  Value get_const(std::size_t address) const;

  const std::vector<std::uint8_t>& get_code() const;

  void dump_data();
  void dump_text();
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
    Pop,

    // Arithmetic
    Negate,
    Add,
    Subtract,
    Multiply,
    Exp,
    Divide,

    // Logical
    Not,
    And,
    Or,
    Equal,
    Greater,
    Less,

    Print,

    LoadNull,

    AllocGlobal,
    StoreGlobal,
    LoadGlobal,

    StoreLocal,
    LoadLocal,

    Jump,
    JumpIfFalse,
  };

  VirtualMachine();
  ~VirtualMachine();

  Value neg(const Value& a);
  Value add(const Value& a, const Value& b);
  Value sub(const Value& a, const Value& b);
  Value mul(const Value& a, const Value& b);
  Value div(const Value& a, const Value& b);
  Value exp(const Value& a, const Value& b);

  Value logical_not(const Value& a);
  Value logical_and(const Value& a, const Value& b);
  Value logical_or(const Value& a, const Value& b);

  Value logical_equals(const Value& a, const Value& b);
  Value logical_greater(const Value& a, const Value& b);
  Value logical_less(const Value& a, const Value& b);

  void alloc_global(const Value& name);
  void store_global(const Value& name, const Value& value);
  void load_global(const Value& name);

  Value execute(const Bytecode* code);

  void halt();
  std::ostream& error();

  void push(Value value);
  Value pop();
private:
  void error(const char* msg);

  bool m_halt = false;
  std::unordered_map<std::string, Value> m_globals;

  const Bytecode* m_code { nullptr };
  const std::uint8_t* m_ip { nullptr };
  std::vector<Value> m_stack;
};
