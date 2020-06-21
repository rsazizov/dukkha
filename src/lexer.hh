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
  MinusEq, StarEq, PlusEq, SlashEq, EqEq, StarStar,

  // Keywords
  Function, Return, Let, Const, For, While, If, Else,

  // Literals
  NumberLiteral, StringLiteral, Null,

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

class Lexer {
public:
  Lexer();
  ~Lexer();

  bool from_file(const char* path);
  bool from_source(const char* source);

  Token next();

private:
  void reset();

  Token keyword_or_identifer();
  Token keyword(std::size_t start, std::size_t len,
      const char* str, TokenType type);
  Token identifer();

  Token string();
  Token number();

  void advance(bool skip_line=false);

  bool is_newline(char ch) const;

  Token match_token(char peek, TokenType match, TokenType mismatch);

  Token make_token(TokenType type);
  Token make_token(double number);
  Token make_token(TokenType type, const char* string);

  char* m_source { nullptr };
  char* m_cursor { nullptr };
  char m_peek { '\0' };

  size_t m_line { 0 };
  size_t m_position { 0 };
};
