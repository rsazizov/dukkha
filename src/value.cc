#include "value.hh"

std::ostream& operator <<(std::ostream& os, ValueType type) {
  switch (type) {
    case ValueType::Number: os << "number"; break;
    case ValueType::Bool: os << "bool"; break;
    case ValueType::Symbol: os << "symbol"; break;
    case ValueType::Object: os << "object"; break;
    case ValueType::String: os << "string"; break;
    case ValueType::Null: os << "null"; break;
    case ValueType::Error: os << "<error>"; break;
  }

  return os;
}

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

Value::Value(const char* str) {
  m_type = ValueType::String;
  m_string = new std::string(str);
}

Value::Value(const std::string& str) {
  m_type = ValueType::String;
  m_string = new std::string(str);
}

Value::~Value() {
  if (is(ValueType::String)) {
    // TODO: Free m_string somehow...
    /* delete m_string; */
  }
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

const std::string& Value::as_string() const {
  return *m_string;
}

std::ostream& operator <<(std::ostream& os, const Value& value) {
  switch (value.getType()) {
    case ValueType::Number: os << value.as_number(); break;
    case ValueType::Bool: os << std::boolalpha << value.as_bool(); break;
    case ValueType::String: os << value.as_string(); break;
    case ValueType::Null: os << "null"; break;
    default: os << "<error>"; break;
  }

  return os;
}
