#include "lexer.hh"

#include <cctype>
#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <cmath>

char* strdup(const char* str) {
  char* dup = new char[std::strlen(str) + 1];
  std::strcpy(dup, str);

  return dup;
}

Token::~Token() {
}

#define PAIR(T) {T, #T}

std::ostream& operator <<(std::ostream& os, TokenType type) {
  os << std::unordered_map<TokenType, const char*> {
    PAIR(TokenType::LeftCurly),
    PAIR(TokenType::RightCurly),
    PAIR(TokenType::LeftSquare),
    PAIR(TokenType::RightSquare),
    PAIR(TokenType::LeftRound),
    PAIR(TokenType::RightRound),
    PAIR(TokenType::Semicolon),
    PAIR(TokenType::Dot),
    PAIR(TokenType::Comma),
    PAIR(TokenType::Minus),
    PAIR(TokenType::Star),
    PAIR(TokenType::Plus),
    PAIR(TokenType::Slash),
    PAIR(TokenType::Eq),
    PAIR(TokenType::MinusEq),
    PAIR(TokenType::StarEq),
    PAIR(TokenType::PlusEq),
    PAIR(TokenType::SlashEq),
    PAIR(TokenType::EqEq),
    PAIR(TokenType::Function),
    PAIR(TokenType::Return),
    PAIR(TokenType::Let),
    PAIR(TokenType::Const),
    PAIR(TokenType::For),
    PAIR(TokenType::While),
    PAIR(TokenType::If),
    PAIR(TokenType::Else),
    PAIR(TokenType::NumberLiteral),
    PAIR(TokenType::StringLiteral),
    PAIR(TokenType::Identifer),
    PAIR(TokenType::Error),
    PAIR(TokenType::EndOfFile),
  }[type];

  return os;
}

Lexer::Lexer() {
}

Lexer::~Lexer() {
  delete [] m_source;
}

bool Lexer::from_file(const char* path) {
  std::ifstream stream(path);

  if (!stream) {
    std::cerr << "Lexer::from_file(): File '" << path << "' does not exist!\n";
    return false;
  }

  stream.seekg(0, stream.end);
  auto size = static_cast<size_t>(stream.tellg());
  stream.seekg(0, stream.beg);

  m_source = new char[size + 1];
  m_source[size] = '\0';

  stream.read(m_source, size);
  stream.close();

  m_position = 0;
  m_line = 1;

  return true;
}

Token Lexer::next() {
  do {
    advance();
  } while (std::isspace(*m_cursor));

  if (std::isalpha(*m_cursor)) {
    return keyword_or_identifer();
  }

  if (std::isdigit(*m_cursor)) {
    return number();
  }

  switch (*m_cursor) {
    case '\0': return make_token(TokenType::EndOfFile);
    case '#': advance(true); return next();
    case '{': return make_token(TokenType::LeftCurly);
    case '}': return make_token(TokenType::RightCurly);
    case '[': return make_token(TokenType::LeftSquare);
    case ']': return make_token(TokenType::RightSquare);
    case '(': return make_token(TokenType::LeftRound);
    case ')': return make_token(TokenType::RightRound);
    case ';': return make_token(TokenType::Semicolon);
    case '.': return make_token(TokenType::Dot);
    case ',': return make_token(TokenType::Comma);
    case '-': return match_token('=', TokenType::MinusEq, TokenType::Minus);
    case '*': return match_token('=', TokenType::StarEq, TokenType::Star);
    case '+': return match_token('=', TokenType::PlusEq, TokenType::Plus);
    case '/': return match_token('=', TokenType::SlashEq, TokenType::Slash);
    case '=': return match_token('=', TokenType::EqEq, TokenType::Eq);
    case '\'': return string();
  }

  return make_token(TokenType::Error);
}

Token Lexer::keyword_or_identifer() {
  switch (*m_cursor) {
    case 'f':
      switch (m_peek) {
        case 'u': return keyword(2, 6, "nction", TokenType::Function);
        case 'o': return keyword(2, 1, "r", TokenType::For);
      }
      break;

    // TODO: Use contexpr for string length?
    case 'l': return keyword(1, 2, "et", TokenType::Let);
    case 'i': return keyword(1, 1, "f", TokenType::If);
    case 'c': return keyword(1, 4, "onst", TokenType::Const);
    case 'w': return keyword(1, 4, "hile", TokenType::While);
    case 'e': return keyword(1, 3, "lse", TokenType::Else);
    case 'r': return keyword(1, 5, "eturn", TokenType::Return);
  }

  return identifer();
}

Token Lexer::keyword(std::size_t start, std::size_t len,
    const char* str, TokenType type) {

  bool match = !std::strncmp(str, m_cursor + start,
      std::strlen(str));

  if (match && std::isspace(*(m_cursor + start + len))) {
    Token token = make_token(type);
    m_cursor += start + len;
    m_position += start + len ;

    return token;
  }

  return identifer();
}

Token Lexer::identifer() {
  const std::size_t MAX_IDENTIFIER_LEN = 32;

  char name[MAX_IDENTIFIER_LEN];
  size_t size = 0;

  while (std::isalpha(*m_cursor) || std::isdigit(*m_cursor)) {
    name[size++] = *m_cursor++;
  }

  m_cursor--;
  name[size] = '\0';

  Token token = make_token(TokenType::Identifer, name);

  m_position += size - 1;

  return token;
}


Token Lexer::string() {
  // Skip "'"
  advance();

  // TODO: Is stringstream the most efficient option?
  std::stringstream ss("");

  while (*m_cursor != '\'') {
    ss << *m_cursor;
    advance();
  }

  return make_token(TokenType::StringLiteral, ss.str().c_str());
}

Token Lexer::number() {
  double value = 0;
  bool decimal = false;
  int n_decimals = 0;

  Token token = make_token(0.0);

  while (std::isdigit(m_peek) || m_peek == '.') {
    if (m_peek == '.') {
      if (decimal) break;

      decimal = true;
      advance();
    }

    if (is_newline(m_peek) || m_peek == '\0') {
      // TODO: ERROR
    }

    double digit = (m_peek) - '0';
    value = 10 * value + digit;

    if (decimal) n_decimals++;

    advance();
  }

  token.as_number = value * pow(10, -n_decimals);
  return token;
}

void Lexer::advance(bool skip_line) {
  if (m_cursor == nullptr) {
    m_cursor = m_source;
  } else {
    if (skip_line) {
      while (!is_newline(m_peek) && m_peek != '\0') {
        advance(false);
      }
    }

    if (is_newline(*m_cursor)) {
      m_line++;
      m_position = 0;
    }

    m_cursor++;
  }

  m_position++;
  m_peek = *(m_cursor + 1);
}

bool Lexer::is_newline(char ch) const {
  return ch == '\n' || ch == '\r';
}

Token Lexer::match_token(char peek, TokenType match, TokenType mismatch) {
  Token token {};

  if (m_peek == peek) {
    token = make_token(match);
    advance();
  } else {
    token = make_token(mismatch);
  }

  return token;
}

Token Lexer::make_token(TokenType type) {
  return (Token) {
    .type = type,
    .line = m_line,
    .position = m_position
  };
}

Token Lexer::make_token(double number) {
  return (Token) {
    .type = TokenType::NumberLiteral,
    .as_number = number,
    .line = m_line,
    .position = m_position
  };
}

Token Lexer::make_token(TokenType type, const char* string) {
  // TODO: assert type is either string literal or ident.
  return (Token) {
    .type = type,
    .as_string = strdup(string),
    .line = m_line,
    .position = m_position
  };
}
