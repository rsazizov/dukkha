#include "lexer.hh"

#include <cctype>
#include <fstream>
#include <iostream>
#include <cstring>

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

  ch = m_source - 1;
  peek = ch + 1;

  stream.read(m_source, size);
  stream.close();

  return true;
}

Token Lexer::next() {
  ch++;

  while (std::isspace(*ch)) {
    ch++;
  }

  peek = ch + 1;

  if (std::isalpha(*ch)) {
    return keyword_or_identifer();
  }

  switch (*ch) {
    case '\0': return make_token(TokenType::EndOfFile);
    case '{': return make_token(TokenType::LeftCurly);
    case '}': return make_token(TokenType::RightCurly);
    case '[': return make_token(TokenType::LeftSquare);
    case ']': return make_token(TokenType::RightSquare);
    case '(': return make_token(TokenType::LeftRound);
    case ')': return make_token(TokenType::RightRound);
    case ';': return make_token(TokenType::Semicolon);
    case '.': return make_token(TokenType::Dot);
    case ',': return make_token(TokenType::Comma);
    case '-':
      return make_token(*peek == '=' ?
          TokenType::MinusEq :
          TokenType::Minus);
    case '*':
      return make_token(*peek == '=' ?
          TokenType::StarEq :
          TokenType::Star);
    case '+':
      return make_token(*peek == '=' ?
          TokenType::PlusEq :
          TokenType::Plus);
    case '/':
      return make_token(*peek == '=' ?
          TokenType::SlashEq :
          TokenType::Slash);
    case '=':
      return make_token(*peek == '=' ?
          TokenType::EqEq :
          TokenType::Eq);
  }

  return make_token(TokenType::Error);
}

Token Lexer::keyword_or_identifer() {
  char peek = *(ch + 1);

  switch (*ch) {
    case 'f':
      switch (peek) {
        case 'u': return keyword(2, 6, "nction", TokenType::Function);
        case 'o': return keyword(2, 1, "r", TokenType::For);
      }
      break;

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

  bool match = !std::strncmp(str, ch + start,
      std::strlen(str));

  if (match && std::isspace(*(ch + start + len))) {
    ch += start + len;
    return make_token(type);
  }

  return identifer();
}

Token Lexer::identifer() {
  char name[32];
  size_t size = 0;

  while (std::isalpha(*ch) || std::isdigit(*ch)) {
    name[size++] = *ch;
    ch++;
  }

  ch--;

  name[size] = '\0';

  return make_token(TokenType::Identifer, name);
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
