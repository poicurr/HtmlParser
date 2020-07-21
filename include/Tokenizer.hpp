#pragma once

#include <ParsingUtility.hpp>
#include <string>

enum class TokenKind {
  Doctype,
  TagBegin,      // <
  TagEnd,        // >
  TagClose,      // </
  TagSelfClose,  // />
  TagName,
  AttributeName,
  AttributeValue,
  Comment,
  Text
};

struct Token {
  TokenKind kind;
  std::string value;
  Token *prev, *next;

  Token(TokenKind kind, const std::string& value)
      : kind{kind}, value{value}, prev{nullptr}, next{nullptr} {}
};

struct TokenList {
  Token *first, *last;
  TokenList() : first{nullptr}, last{nullptr} {}
  void add(TokenKind kind, const std::string& value) {
    Token* newToken = new Token(kind, value);
    newToken->prev = last;
    newToken->next = nullptr;
    if (newToken->prev) newToken->prev->next = newToken;
    if (!first) first = newToken;
    last = newToken;
  }
  Token* begin() { return first; }
  Token* end() { return nullptr; }
};

// -----------------------------------
// tokenizer state
// -----------------------------------
struct ParserState {
  bool isQuoted;
  bool isEscaped;

  ParserState() : isQuoted{false}, isEscaped{false} {}
};

ParserState state;

// -----------------------------------
// tokenizer
// -----------------------------------
void tokenizeDoctype(TokenList& tokenList, const char*& p) {
  tokenList.add(TokenKind::TagBegin, "<!doctype");

  skipWs(p);
  const char* p0 = p;
  while (!startWith(p, ">")) {
    ++p;
  }
  tokenList.add(TokenKind::Doctype, std::string{p0, p});

  if (*p == '>') {
    p += 1;
    tokenList.add(TokenKind::TagEnd, ">");
  }
}

void tokenizeText(TokenList& tokenList, const char*& p) {
  skipWs(p);

  if (*p == '<') {
    return;
  }

  const char* p0 = p;
  while (*p != '<' && *p != '\0') {
    ++p;
  }
  auto text = std::string{p0, p};
  if (!text.empty()) {
    tokenList.add(TokenKind::Text, trim(text));
  }
}

void tokenizeTagOpen(TokenList& tokenList, const char*& p) {
  tokenList.add(TokenKind::TagBegin, "<");
  skipWs(p);

  // tagname
  const char* p0 = p;
  while (isLetter(*p)) {
    ++p;
  }
  std::string tagName = {p0, p};
  tokenList.add(TokenKind::TagName, tagName);

  // attributes
  while (!startWith(p, "/>") && *p != '>') {
    skipWs(p);

    // attribute name
    const char* p1 = p;
    while (isLetter(*p)) {
      ++p;
    }
    std::string attributeName = {p1, p};
    if (!attributeName.empty()) tokenList.add(TokenKind::AttributeName, attributeName);

    skipWs(p);

    // attribute value
    if (*p == '=') {
      ++p;
      skipWs(p);
      if (*p == '"') {
        ++p;
        const char* p2 = p;
        while (*p != '"') {
          ++p;
        }
        std::string attributeValue = {p2, p};
        if (!attributeValue.empty()) tokenList.add(TokenKind::AttributeValue, attributeValue);
      }
    }
    ++p;
  }

  // self closing tag
  if (startWith(p, "/>")) {
    p += 2;
    tokenList.add(TokenKind::TagSelfClose, "/>");
  }

  if (*p == '>') {
    p += 1;
    tokenList.add(TokenKind::TagEnd, ">");
  }
}

void tokenizeTagClose(TokenList& tokenList, const char*& p) {
  tokenList.add(TokenKind::TagClose, "</");
  skipWs(p);

  // tagname
  const char* p0 = p;
  while (isLetter(*p)) {
    ++p;
  }
  std::string tagName = {p0, p};
  tokenList.add(TokenKind::TagName, tagName);

  skipWs(p);

  if (*p == '>') {
    tokenList.add(TokenKind::TagEnd, ">");
    p += 1;
  }
}

void tokenizeComment(TokenList& tokenList, const char*& p) {
  const char* p0 = p;
  while (!startWith(p, "-->")) {
    ++p;
  }
  std::string comment = {p0, p};
  tokenList.add(TokenKind::Comment, comment);
  if (startWith(p, "-->")) {
    p += 3;
  }
}

TokenList tokenize(const char* p) {
  TokenList tokenList;
  while (*p != '\0') {
    skipWs(p);

    if (startWith(p, "<!doctype")) {
      // doctype
      p += 9;
      tokenizeDoctype(tokenList, p);
      tokenizeText(tokenList, p);
      continue;
    }

    if (startWith(p, "<!--")) {
      // comment
      p += 4;
      tokenizeComment(tokenList, p);
      tokenizeText(tokenList, p);
      continue;
    }

    if (startWith(p, "</")) {
      // tag close
      p += 2;
      tokenizeTagClose(tokenList, p);
      tokenizeText(tokenList, p);
      continue;
    }

    if (*p == '<') {
      // tag open
      p += 1;
      tokenizeTagOpen(tokenList, p);
      tokenizeText(tokenList, p);
      continue;
    }

    ++p;
  }
  return tokenList;
}
