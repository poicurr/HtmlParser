#pragma once

#include <string>

// -----------------------------------
// parsing utility
// -----------------------------------
char toLower(char c) { return 'A' <= c && c <= 'Z' ? 'a' - 'A' + c : c; }
bool isAlpha(char c) { return 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z'; }
bool isDigit(char c) { return '0' <= c && c <= '9'; }
bool isSpace(char c) { return c == ' ' || c == '\r' || c == '\n' || c == '\t'; }
bool isLetter(char c) { return isAlpha(c) || isDigit(c) || c == '_'; }
bool startWith(const char* p, const std::string& s) {
  const std::size_t len = s.size();
  for (int i = 0; i < len; ++i) {
    if (p[i] != s[i]) return false;
  }
  return true;
}

void skipWs(const char*& p) {
  while (isSpace(*p)) ++p;
}

template <class Predicate>
std::string readUntil(const char* p, Predicate&& pred) {
  const char* p0 = p;
  while (!pred(*p)) ++p;
  return {p0, p};
}

// helper
std::string trimLeft(const std::string& s) {
  auto first = s.begin(), last = s.end();
  while (isSpace(*first) && first != last) {
    ++first;
  }
  return {first, last};
}

std::string trimRight(const std::string& s) {
  auto first = s.begin(), last = first + s.size() - 1;
  while (isSpace(*last) && first != last) {
    --last;
  }
  return {first, last + 1};
}

std::string trim(const std::string& s) { return trimLeft(trimRight(s)); }
