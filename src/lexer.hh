#pragma once

#include <ostream>
#include <vector>
#include <string>
#include <unordered_map>

enum class TokenType {
  // 1 character tokens
  LeftCurly, RightCurly, LeftSquare, RightSquare, LeftRound, RightRound,
  Semicolon, Dot, Comma, Minus, Star, Plus, Slash, Eq,

  // 2 character tokens
  MinusEq, StarEq, PlusEq, SlashEq, EqEq,

  // Keywords
  Function, Return, Let, Const, For, While, If, Else,

  // Literals
  NumberLiteral, StringLiteral,

  Identifer,

  // Special tokens
  Error, EndOfFile
};

std::ostream& operator <<(std::ostream& os, TokenType type);

struct Token {
  Token() = default;
  ~Token();

  TokenType type { TokenType::EndOfFile };

  union {
    const char* as_string;
    double as_number { 0 };
  };

  std::size_t line { 0 };
  std::size_t position { 0 };
};

std::ostream& operator << (std::ostream& os, const Token& token);

class Lexer {
public:
  Lexer();
  ~Lexer();

  bool from_file(const char* path);
  Token next();

private:
  Token keyword_or_identifer();
  Token keyword(std::size_t start, std::size_t len,
      const char* str, TokenType type);
  Token identifer();

  Token make_token(TokenType type);
  Token make_token(double number);
  Token make_token(TokenType type, const char* string);

  char* m_source { nullptr };
  char* ch { nullptr };
  char* peek { nullptr };

  size_t m_line { 0 };
  size_t m_position { 0 };
};
