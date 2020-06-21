#pragma once

#include <ostream>
#include <string>

enum class ValueType : std::uint8_t {
  // TODO: float and int
  Number,
  Bool,
  Symbol,
  Object,
  String,

  // Used internally.
  Error
};

std::ostream& operator <<(std::ostream& os, ValueType type);

class Value {
public:
  Value(ValueType type);
  Value(double value);
  Value(bool value);
  Value(const char* str);
  Value(const std::string& str);

  ~Value();

  bool is(ValueType type) const;

  ValueType getType() const;

  double as_number() const;
  bool as_bool() const;
  const std::string& as_string() const;
private:
  ValueType m_type;

  union {
    double m_number;
    bool m_bool;
    std::string* m_string { nullptr };
  };
};

std::ostream& operator <<(std::ostream& os, const Value& value);
