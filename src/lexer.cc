#include "lexer.hh"

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

#define TT_CASE(os, x) case TokenType::x: os << #x; break;

std::ostream& operator <<(std::ostream& os, TokenType type) {
  switch (type) {
    TT_CASE(os, LeftCurly)
    TT_CASE(os, RightCurly)
    TT_CASE(os, LeftSquare)
    TT_CASE(os, RightSquare)
    TT_CASE(os, LeftRound)
    TT_CASE(os, RightRound)
    TT_CASE(os, Semicolon)
    TT_CASE(os, Dot)
    TT_CASE(os, Comma)
    TT_CASE(os, Minus)
    TT_CASE(os, Star)
    TT_CASE(os, Plus)
    TT_CASE(os, Slash)
    TT_CASE(os, Eq)
    TT_CASE(os, Less)
    TT_CASE(os, Greater)
    TT_CASE(os, Bang)
    TT_CASE(os, MinusEq)
    TT_CASE(os, StarEq)
    TT_CASE(os, PlusEq)
    TT_CASE(os, SlashEq)
    TT_CASE(os, EqEq)
    TT_CASE(os, StarStar)
    TT_CASE(os, LessEq)
    TT_CASE(os, GreaterEq)
    TT_CASE(os, BangEq)
    TT_CASE(os, Function)
    TT_CASE(os, Return)
    TT_CASE(os, Let)
    TT_CASE(os, Const)
    TT_CASE(os, For)
    TT_CASE(os, While)
    TT_CASE(os, If)
    TT_CASE(os, Else)
    TT_CASE(os, And)
    TT_CASE(os, Or)
    TT_CASE(os, Not)
    TT_CASE(os, True)
    TT_CASE(os, False)
    TT_CASE(os, Print)
    TT_CASE(os, NumberLiteral)
    TT_CASE(os, StringLiteral)
    TT_CASE(os, Null)
    TT_CASE(os, Identifer)
    TT_CASE(os, Error)
    TT_CASE(os, EndOfFile)
    default:
      os << "<error>";
  }

  return os;
}

#undef TT_CASE

Lexer::Lexer() {
}

Lexer::~Lexer() {
  delete [] m_source;
}

void Lexer::reset() {
  delete [] m_source;

  m_position = 0;
  m_line = 1;

  m_peek = '\0';
  m_cursor = nullptr;
}

bool Lexer::from_file(const char* path) {
  reset();

  std::ifstream stream(path);

  if (!stream) {
    std::cerr << "Lexer::from_file(): File '" << path << "' does not exist!\n";
    stream.close();
    return false;
  }

  stream.seekg(0, stream.end);
  auto size = static_cast<size_t>(stream.tellg());
  stream.seekg(0, stream.beg);

  m_source = new char[size + 1];
  m_source[size] = '\0';

  stream.read(m_source, size);
  stream.close();

  return true;
}

bool Lexer::from_source(const char* source) {
  reset();
  m_source = strdup(source);
  // TODO: ??
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
    case '*': {
      advance();
      switch (*m_cursor) {
        case '=': return make_token(TokenType::StarEq);
        case '*': return make_token(TokenType::StarStar);
        default: return make_token(TokenType::Star);
      }
    }
    case '+': return match_token('=', TokenType::PlusEq, TokenType::Plus);
    case '<': return match_token('=', TokenType::LessEq, TokenType::Less);
    case '>': return match_token('=', TokenType::GreaterEq, TokenType::Greater);
    case '/': return match_token('=', TokenType::SlashEq, TokenType::Slash);
    case '!': return match_token('=', TokenType::BangEq, TokenType::Bang);
    case '=': return match_token('=', TokenType::EqEq, TokenType::Eq);
    case '\'': return string();
  }

  return make_token(TokenType::Error, "Unexpected symbol");
}

Token Lexer::keyword_or_identifer() {
  switch (*m_cursor) {
    case 'a': return keyword(1, 2, "nd", TokenType::And);
    case 'o': return keyword(1, 1, "r", TokenType::Or);
    case 't': return keyword(1, 3, "rue", TokenType::True);
    case 'f':
      switch (m_peek) {
        case 'a': return keyword(2, 3, "lse", TokenType::False);
        case 'u': return keyword(2, 6, "nction", TokenType::Function);
        case 'o': return keyword(2, 1, "r", TokenType::For);
      }
      break;
    case 'l': return keyword(1, 2, "et", TokenType::Let);
    case 'i': return keyword(1, 1, "f", TokenType::If);
    case 'c': return keyword(1, 4, "onst", TokenType::Const);
    case 'p': return keyword(1, 4, "rint", TokenType::Print);
    case 'w': return keyword(1, 4, "hile", TokenType::While);
    case 'e': return keyword(1, 3, "lse", TokenType::Else);
    case 'n': {
      switch (m_peek) {
        case 'u': return keyword(2, 3, "ull", TokenType::Null);
        case 'o': return keyword(2, 1, "t", TokenType::Not);
      }
      break;
    }
    case 'r': return keyword(1, 5, "eturn", TokenType::Return);
  }

  return identifer();
}

Token Lexer::keyword(std::size_t start, std::size_t len,
    const char* str, TokenType type) {

  bool match = !std::strncmp(str, m_cursor + start,
      std::strlen(str));

  char after = *(m_cursor + start + len);
  std::cout << after << "\n";
  if (match && !std::isalpha(after) && !std::isdigit(after) && after != '_') {
    Token token = make_token(type);
    m_cursor += start + len - 1;
    m_position += start + len - 1;

    return token;
  }

  return identifer();
}

Token Lexer::identifer() {
  const std::size_t MAX_IDENTIFIER_LEN = 32;

  char name[MAX_IDENTIFIER_LEN];
  size_t size = 0;

  while (std::isalpha(*m_cursor) || std::isdigit(*m_cursor) || *m_cursor == '_') {
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
    if (is_newline(*m_cursor) || *m_cursor == '\0') {
      return make_token(TokenType::Error, "Unexpected end of line/file");
    }

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

  while (std::isdigit(*m_cursor) || *m_cursor == '.') {
    if (*m_cursor == '.') {
      if (decimal) break;

      decimal = true;
      advance();
    }

    double digit = (*m_cursor) - '0';
    value = 10 * value + digit;

    if (decimal) n_decimals++;

    advance();
  }

  m_peek = *(--m_cursor);
  m_position--;

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
