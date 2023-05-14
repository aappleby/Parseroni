#pragma once

#include "parseroni/PNodes.h"

#include "metrolib/core/Result.h"

#include <assert.h>
#include <stack>
#include <string>
#include <stdint.h>
#include <optional>
#include <functional>

struct PNode;

//------------------------------------------------------------------------------

struct PInt {
  PInt() : is_negative(false), u_int(0) {}

  PInt(cspan span, bool is_negative, uint64_t b) {
    this->span = span;
    this->is_negative = is_negative;
    this->u_int = b;
    if (is_negative) {
      assert(b <= 0x8000000000000000);
    }
  }

  cspan span;
  union {
    uint64_t u_int;
    int64_t  s_int;
  };
  bool is_negative;
};

//------------------------------------------------------------------------------

class Parser {
public:

  Parser() {}
  void load(const std::string& text);

  //----------------------------------------

  typedef std::function<std::optional<cspan>()> taker;

  std::optional<cspan> take_some(taker t);

  std::optional<cspan> take(const char* text);
  std::optional<cspan> take_token();
  std::optional<cspan> take_number();
  std::optional<cspan> take_string();
  std::optional<cspan> take_digraph();
  std::optional<cspan> take_punct();

  std::optional<cspan> take(char c);
  std::optional<cspan> take_until(char c, int min = 0);
  std::optional<cspan> take_delimited(const char* prefix, const char* suffix, const char* escape = nullptr);

  std::optional<cspan> take_digit(int base);
  std::optional<cspan> take_digits(int base, int len);
  std::optional<cspan> take_digits(int base);

  std::optional<cspan> take_constant_expression();
  std::optional<cspan> take_declaration_specifier();
  std::optional<cspan> take_declaration();
  std::optional<cspan> take_declarator();
  std::optional<cspan> take_enum_specifier();
  std::optional<cspan> take_enumerator_list();
  std::optional<cspan> take_enumerator();
  std::optional<cspan> take_escape_seq();
  std::optional<cspan> take_identifier();
  std::optional<cspan> take_include_path();
  std::optional<cspan> take_int();
  std::optional<cspan> take_pointer();
  std::optional<cspan> take_primitive_type();
  std::optional<cspan> take_signed_specifier();
  std::optional<cspan> take_specifier_qualifier();
  std::optional<cspan> take_storage_class_specifier();
  std::optional<cspan> take_str();
  std::optional<cspan> take_struct_declaration();
  std::optional<cspan> take_struct_declarator_list();
  std::optional<cspan> take_struct_declarator();
  std::optional<cspan> take_struct_or_union_specifier();
  std::optional<cspan> take_type_qualifier();
  std::optional<cspan> take_type_specifier();
  std::optional<cspan> take_typedef_name();
  std::optional<cspan> take_ws_opt();
  std::optional<cspan> take_ws();


  std::optional<PInt>  take_int_as_pint();

  //PPreproc* take_preproc();
  std::optional<PPreproc*> take_preproc();
  std::optional<PTranslationUnit*> take_translation_unit();

  // PNode* take_oneof(taker, ...);

  //----------------------------------------

  static bool parse_digits(const char* s, int base, uint64_t& out);
  static bool parse_int(cspan s, PInt& out);

  //----------------------------------------

  std::string source;
  const char* source_start = nullptr;
  const char* source_end = nullptr;

  //----------------------------------------

  std::string ws;

  //----------------------------------------

  const char* cursor;
  std::stack<const char*> cursor_stack;

  cspan take_top_span() {
    assert(!cursor_stack.empty());
    cspan result(cursor_stack.top(), cursor);
    cursor_stack.pop();
    return result;
  }

  std::optional<cspan> drop_span() {
    pop_cursor();
    return std::nullopt;
  }

  cspan top_span() {
    assert(!cursor_stack.empty());
    auto result = cspan(cursor_stack.top(), cursor);
    return result;
  }

  void start_span() {
    cursor_stack.push(cursor);
  }

  void pop_cursor() {
    assert (cursor_stack.size());
    cursor = cursor_stack.top();
    cursor_stack.pop();
  }

  void keep_cursor() {
    cursor_stack.pop();
  }
};

//------------------------------------------------------------------------------
