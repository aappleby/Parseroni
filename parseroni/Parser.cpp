#include "parseroni/Parser.h"

#include "metrolib/core/Log.h"

#include <memory>
#include <string.h>

using std::make_unique;

//taker taker = [this]() { return take("asdf"); };

bool isglyph(char c) {
  if (c >= '0' && c <= '9') return true;
  if (c >= 'a' && c <= 'z') return true;
  if (c >= 'A' && c <= 'Z') return true;
  return c == '_';
}

/*
All punctuations in C:
! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~
*/

void log_span(cspan s, uint32_t color) {
  char buf[256] = {0};
  memcpy(buf, s.data(), s.size());
  buf[s.size()] = 0;
  LOG_C(color, buf);
}

//------------------------------------------------------------------------------

void Parser::load(const std::string& text) {
  source = text;

  while(source.size() && source.back() == 0) source.pop_back();
  source.push_back(0);

  source_start = source.data();
  source_end = source.data() + source.size() - 1;
  assert(source_end[0]  == 0);

  cursor = source_start;
  // why does stack have no clear();
  cursor_stack = std::stack<const char*>();
}

//------------------------------------------------------------------------------

bool char_to_digit(char c, int base, int& out) {
  if (base == 16) {
    if (c >= '0' && c <= '9') {
      out = c - '0';
      return true;
    }
    else if (c >= 'a' && c <= 'f') {
      out = c - 'a' + 10;
      return true;
    }
    else if (c >= 'A' && c <= 'F') {
      out = c - 'A' + 10;
      return true;
    }
    else {
      return false;
    }
  }

  if (c >= '0' && c <= ('0' + base - 1)) {
    out = c - '0';
    return true;
  }
  else {
    return false;
  }
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_digit(int base) {
  start_span();

  auto c = *cursor++;

  if (base == 16) {
    if (c >= '0' && c <= '9') {
      return take_top_span();
    }
    else if (c >= 'a' && c <= 'f') {
      return take_top_span();
    }
    else if (c >= 'A' && c <= 'F') {
      return take_top_span();
    }
    else {
      return std::nullopt;
    }
  }

  if (c >= '0' && c <= ('0' + base - 1)) {
    return take_top_span();
  }
  else {
    return std::nullopt;
  }
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take(const char* text) {
  if (*text == 0) return std::nullopt;

  start_span();

  while(1) {
    if (*text == 0) return take_top_span();
    if (cursor == source_end) return drop_span();
    if (*cursor != *text) return drop_span();

    cursor++;
    text++;
  }

  return drop_span();
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take(char c) {
  if (*cursor == c) {
    start_span();
    cursor++;
    return take_top_span();
  }
  else {
    return std::nullopt;
  }
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_until(char c, int min) {
  start_span();

  auto old_cursor = cursor;

  while (cursor < source_end && *cursor != c) cursor++;

  if ((*cursor == c) && (top_span().size() >= min)) {
    return take_top_span();
  }
  else {
    return drop_span();
  }
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_delimited(const char* prefix, const char* suffix, const char* escape) {
  start_span();

  if (!take(prefix)) return drop_span();

  for (;cursor < source_end; cursor++) {
    if (escape && take(escape)) {
      if (!take_escape_seq().has_value()) return drop_span();
    }
    if (take(suffix)) {
      return take_top_span();
    }
  }

  return drop_span();
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_digits(int base, int count) {
  if (base == 0) return std::nullopt;

  start_span();

  int digits = 0;
  for (;*cursor && cursor < source_end; cursor++) {
    int digit = 0;
    if (!char_to_digit(*cursor, base, digit)) break;
    digits++;
  }

  if (digits == count) {
    return take_top_span();
  }
  else {
    return drop_span();
  }
}

std::optional<cspan> Parser::take_digits(int base) {
  if (base == 0) return std::nullopt;

  start_span();

  int digits = 0;
  for (;*cursor && cursor < source_end; cursor++) {
    int digit = 0;
    if (!char_to_digit(*cursor, base, digit)) break;
    digits++;
  }

  if (digits == 0) {
    return drop_span();
  }
  else {
    return take_top_span();
  }

}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_ws() {
  start_span();
  while(isspace(*cursor) && cursor < source_end) cursor++;

  if (top_span().empty()) {
    return drop_span();
  }
  else {
    return take_top_span();
  }
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_str() {
  return take_delimited("\"", "\"", "\\");
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_some(Parser::taker t) {
  start_span();
  int count = 0;

  while(t()) {
    count++;
  }

  if (count) {
    return take_top_span();
  }
  else {
    return std::nullopt;
  }
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_int() {
  start_span();

  // Consume a leading minus sign if present.
  if (take('-')) {
    // FIXME should whitespace be allowed between the minus and the number?
    // Answering yes for now.
    take_ws();
  }

  if (take("0x")) {
    // Hexadecimal
    if (take_digits(16)) {
      return take_top_span();
    }
    else {
      return drop_span();
    }
  }
  else if (take("0b")) {
    // Binary
    if (take_digits(2)) {
      return take_top_span();
    }
    else {
      return drop_span();
    }
  }
  else if (take("0")) {
    // Octal or bare zero
    take_digits(8);
    return take_top_span();
  }
  else if (take_digits(10)) {
    // Decimal
    return take_top_span();
  }
  else {
    return drop_span();
  }



#if 0
  if (!isdigit(*cursor)) {
    return drop_span();
  }

  if (*cursor != '0') {
    // starts with [1-9]
    base = 10;
  }
  else if (isdigit(cursor[1])) {
    // starts with 0[1-9]
    cursor += 1;
    base = 8;
  }
  else if (cursor[1] == 'b') {
    // starts with 0b
    cursor += 2;
    base = 2;
  }
  else if (cursor[1] == 'x') {
    // starts with 0x
    cursor += 2;
    base = 16;
  }
  else {
    // bare 0 followed by punctuation
    return take_top_span();
  }

  if (base && take_digits(base)) {
    return take_top_span();
  }
  else {
    return drop_span();
  }
#endif
}

//------------------------------------------------------------------------------

std::optional<PInt> Parser::take_int_as_pint() {
  start_span();

  int base = 0;
  uint64_t accum = 0;

  bool negative = *cursor == '-';
  if (negative) cursor++;

  if (!isdigit(*cursor)) {
    return drop_span();
  }

  if (*cursor != '0') {
    // starts with [1-9]
    base = 10;
  }
  else if (isdigit(cursor[1])) {
    // starts with 0[1-9]
    cursor += 1;
    base = 8;
  }
  else if (cursor[1] == 'b') {
    // starts with 0b
    cursor += 2;
    base = 2;
  }
  else if (cursor[1] == 'x') {
    // starts with 0x
    cursor += 2;
    base = 16;
  }
  else if ((ispunct(cursor[1]) && cursor[1] != '_') || cursor[1] == 0) {
    // bare 0 followed by punctuation
    return PInt(take_top_span(), negative, 0);
  }
  else {
    // bad prefix
    return drop_span();
  }

  if (base && parse_digits(base, accum)) {
    if (negative && accum > 0x8000000000000000) {
      return drop_span();
    }
    return PInt(take_top_span(), negative, accum);
  }
  else {
    return drop_span();
  }
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_escape_seq() {
  start_span();

  switch (*cursor) {
    case '\'':
    case '"':
    case '?':
    case '\\':
    case '\a':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      take(*cursor);
      return take_top_span();
  }

  auto bookmark = cursor;

  // \nnn
  if (take_digits(8, 3)) return take_top_span();
  else cursor = bookmark;

  // \o{n..}
  if (take("o{") && take_digits(8) && take("}")) return take_top_span();
  else cursor = bookmark;

  // \xn..
  if (take("x") && take_digits(16)) return take_top_span();
  else cursor = bookmark;

  // \x{n..}
  if (take("x{") && take_digits(16) && take("}")) return take_top_span();
  else cursor = bookmark;

  // \unnnn
  if (take("u") && take_digits(16, 4)) return take_top_span();
  else cursor = bookmark;

  // \u{n..}
  if (take("u{") && take_digits(16) && take("}")) return take_top_span();
  else cursor = bookmark;

  // \Unnnnnnnn
  if (take("U") && take_digits(16, 8)) return take_top_span();
  else cursor = bookmark;

  // \N{name}
  if (take("N{") && take_until('}', 1) && take('}')) return take_top_span();
  else cursor = bookmark;

  return drop_span();
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_include_path() {
  if (*cursor == '"') return take_delimited("\"", "\"");
  if (*cursor == '<') return take_delimited("<", ">");
  return std::nullopt;
}

//------------------------------------------------------------------------------

std::optional<PPreproc*> Parser::take_preproc() {
  if (*cursor != '#') return std::nullopt;

  start_span();

  auto lit_include = take("#include");
  if (!lit_include) { return drop_span(); }

  auto lit_ws = take_ws();
  if (!lit_ws) { return drop_span(); }

  auto lit_path = take_include_path();
  if (!lit_path) { return drop_span(); }

  auto result = new PPreprocInclude();
  result->span = take_top_span();
  result->lit_include = lit_include.value();
  result->lit_ws = lit_ws.value();
  result->lit_path = lit_path.value();

  return result;
}

//------------------------------------------------------------------------------

/*
<translation-unit> ::= {<external-declaration>}*

<external-declaration> ::= <function-definition>
                         | <declaration>

<declaration> ::=  {<declaration-specifier>}+ {<init-declarator>}* ;

*/

std::optional<PTranslationUnit*> Parser::take_translation_unit() {
  start_span();

  auto result = std::make_unique<PTranslationUnit>();

  while(cursor != source_end) {
    if (auto ws = take_ws()) {
      auto ws_node = std::make_unique<PNode>();
      ws_node->span = ws.value();
      result->children.push_back(ws_node.release());
    }
  }

  result->span = take_top_span();

  return result.release();
}

//------------------------------------------------------------------------------

bool Parser::parse_digits(int base, uint64_t& out) {
  start_span();

  uint64_t accum = 0;
  int digits = 0;

  while (*cursor && cursor < source_end) {
    int digit = 0;
    if (char_to_digit(*cursor, base, digit)) {
      uint64_t old_accum = accum;
      accum = accum * base + digit;
      if (accum < old_accum) return false;
    }
    else {
      break;
    }
    digits++;
    cursor++;
  }

  if (isalpha(*cursor) || digits == 0) {
    pop_cursor();
    return false;
  }

  out = accum;
  keep_cursor();
  return true;
}

//------------------------------------------------------------------------------
