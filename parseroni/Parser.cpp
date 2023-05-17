#include "parseroni/Parser.h"

#include "parseroni/Combinators.h"

#include "metrolib/core/Log.h"

#include <memory>
#include <string.h>
#include <assert.h>

using namespace matcheroni;

using std::make_unique;

/*
The syntax of C in Backus-Naur Form

<external-declaration> ::= <function-definition>
                         | <declaration>

<function-definition> ::= {<declaration-specifier>}* <declarator> {<declaration>}* <compound-statement>

<declaration-specifier> ::= <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier>

<storage-class-specifier> ::= auto
                            | register
                            | static
                            | extern
                            | typedef


<struct-or-union> ::= struct
                    | union

<struct-declaration> ::= {<specifier-qualifier>}* <struct-declarator-list>

<specifier-qualifier> ::= <type-specifier>
                        | <type-qualifier>

<struct-declarator-list> ::= <struct-declarator>
                           | <struct-declarator-list> , <struct-declarator>

<direct-declarator> ::= <identifier>
                      | ( <declarator> )
                      | <direct-declarator> [ {<constant-expression>}? ]
                      | <direct-declarator> ( <parameter-type-list> )
                      | <direct-declarator> ( {<identifier>}* )

<constant-expression> ::= <conditional-expression>

<conditional-expression> ::= <logical-or-expression>
                           | <logical-or-expression> ? <expression> : <conditional-expression>

<logical-or-expression> ::= <logical-and-expression>
                          | <logical-or-expression> || <logical-and-expression>

<logical-and-expression> ::= <inclusive-or-expression>
                           | <logical-and-expression> && <inclusive-or-expression>

<inclusive-or-expression> ::= <exclusive-or-expression>
                            | <inclusive-or-expression> | <exclusive-or-expression>

<exclusive-or-expression> ::= <and-expression>
                            | <exclusive-or-expression> ^ <and-expression>

<and-expression> ::= <equality-expression>
                   | <and-expression> & <equality-expression>

<equality-expression> ::= <relational-expression>
                        | <equality-expression> == <relational-expression>
                        | <equality-expression> != <relational-expression>

<relational-expression> ::= <shift-expression>
                          | <relational-expression> < <shift-expression>
                          | <relational-expression> > <shift-expression>
                          | <relational-expression> <= <shift-expression>
                          | <relational-expression> >= <shift-expression>

<shift-expression> ::= <additive-expression>
                     | <shift-expression> << <additive-expression>
                     | <shift-expression> >> <additive-expression>

<additive-expression> ::= <multiplicative-expression>
                        | <additive-expression> + <multiplicative-expression>
                        | <additive-expression> - <multiplicative-expression>

<multiplicative-expression> ::= <cast-expression>
                              | <multiplicative-expression> * <cast-expression>
                              | <multiplicative-expression> / <cast-expression>
                              | <multiplicative-expression> % <cast-expression>

<cast-expression> ::= <unary-expression>
                    | ( <type-name> ) <cast-expression>

<unary-expression> ::= <postfix-expression>
                     | ++ <unary-expression>
                     | -- <unary-expression>
                     | <unary-operator> <cast-expression>
                     | sizeof <unary-expression>
                     | sizeof <type-name>

<postfix-expression> ::= <primary-expression>
                       | <postfix-expression> [ <expression> ]
                       | <postfix-expression> ( {<assignment-expression>}* )
                       | <postfix-expression> . <identifier>
                       | <postfix-expression> -> <identifier>
                       | <postfix-expression> ++
                       | <postfix-expression> --

<primary-expression> ::= <identifier>
                       | <constant>
                       | <string>
                       | ( <expression> )

<constant> ::= <integer-constant>
             | <character-constant>
             | <floating-constant>
             | <enumeration-constant>

<expression> ::= <assignment-expression>
               | <expression> , <assignment-expression>

<assignment-expression> ::= <conditional-expression>
                          | <unary-expression> <assignment-operator> <assignment-expression>

<assignment-operator> ::= = | *= | /= | %= | += | -= | <<= | >>= | &=| ^= | |=
<unary-operator> ::= & | * | + | - | ~ | !

<type-name> ::= {<specifier-qualifier>}+ {<abstract-declarator>}?

<parameter-type-list> ::= <parameter-list> | <parameter-list> , ...

<parameter-list> ::= <parameter-declaration> | <parameter-list> , <parameter-declaration>

<parameter-declaration> ::= {<declaration-specifier>}+ <declarator>
                          | {<declaration-specifier>}+ <abstract-declarator>
                          | {<declaration-specifier>}+

<abstract-declarator> ::= <pointer>
                        | <pointer> <direct-abstract-declarator>
                        | <direct-abstract-declarator>

<direct-abstract-declarator> ::=  ( <abstract-declarator> )
                               | {<direct-abstract-declarator>}? [ {<constant-expression>}? ]
                               | {<direct-abstract-declarator>}? ( {<parameter-type-list>}? )

<enum-specifier> ::= enum <identifier> { <enumerator-list> }
                   | enum { <enumerator-list> }
                   | enum <identifier>


<enumerator> ::= <identifier>
               | <identifier> = <constant-expression>

<typedef-name> ::= <identifier>

<declaration> ::=  {<declaration-specifier>}+ {<init-declarator>}* ;

<init-declarator> ::= <declarator>
                    | <declarator> = <initializer>

<initializer> ::= <assignment-expression>
                | { <initializer-list> }
                | { <initializer-list> , }

<initializer-list> ::= <initializer>
                     | <initializer-list> , <initializer>

<compound-statement> ::= { {<declaration>}* {<statement>}* }

<statement> ::= <labeled-statement>
              | <expression-statement>
              | <compound-statement>
              | <selection-statement>
              | <iteration-statement>
              | <jump-statement>

<labeled-statement> ::= <identifier> : <statement>
                      | case <constant-expression> : <statement>
                      | default : <statement>

<expression-statement> ::= {<expression>}? ;

<selection-statement> ::= if ( <expression> ) <statement>
                        | if ( <expression> ) <statement> else <statement>
                        | switch ( <expression> ) <statement>

<iteration-statement> ::= while ( <expression> ) <statement>
                        | do <statement> while ( <expression> ) ;
                        | for ( {<expression>}? ; {<expression>}? ; {<expression>}? ) <statement>

<jump-statement> ::= goto <identifier> ;
                   | continue ;
                   | break ;
                   | return {<expression>}? ;

This grammar was adapted from Section A13 of The C programming language, 2nd edition, by Brian W. Kernighan and Dennis M. Ritchie,Prentice Hall, 1988.
*/

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
  memcpy(buf, s.begin, s.size());
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

const char* chars_graph = "!\"#%&'()*+,-./:;<=>?[\\]^_{|}~";

//------------------------------------------------------------------------------

#if 0

node take_identifier() {
  return one(_a-zA-Z) + any(_a-zA-A);
}

#endif

/*
Lexical elements[15]==
  Identifiers[16]
  Floating constants[44]
  Integer constants[48]
  Character constants[52]
  String literals[53]
  Comments[55]

token:
  keyword
  identifier
  constant
  string-literal
  punctuator
preprocessing-token:
  header-name
  identifier
  pp-number
  character-constant
  string-literal
  punctuator
  each non-white-space character that cannot be one of the above

keywords:
  auto  break  case  char  const  continue  default  do  double  else  enum
  extern  float  for  goto  if  inline  int  long  register  restrict  return
  short  signed  sizeof  static  struct  switch  typedef  union  unsigned  void
  volatile  while  _Alignas  _Alignof  _Atomic  _Bool  _Complex  _Generic
  _Imaginary  _Noreturn  _Static_assert  _Thread_local
*/

std::optional<cspan> Parser::take_token() {
  const char* end = nullptr;

  if (end = match_identifier(cursor)) {
    return take_span(end);
  }
  else if (end = match_int(cursor)) {
    return take_span(end);
  }
  else if (end = match_float(cursor)) {
    return take_span(end);
  }
  else if (end = match_preproc(cursor)) {
    return take_span(end);
  }
  else if (end = match_string(cursor)) {
    return take_span(end);
  }
  else if (end = match_punct(cursor)) {
    return take_span(end);
  }

  return std::nullopt;
}

//------------------------------------------------------------------------------

#if 0
node take_enumerator() {
  return (take_enum_constant() + take_("=") + take_constant_expression()) ||
         take_enum_constant();
}

#endif

//------------------------------------------------------------------------------

#if 0
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
#endif

//------------------------------------------------------------------------------

#if 0
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
#endif

//------------------------------------------------------------------------------

#if 0
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
#endif

//------------------------------------------------------------------------------

#if 0
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
#endif

//------------------------------------------------------------------------------

#if 0
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
#endif

//------------------------------------------------------------------------------

#if 0
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
#endif

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take_ws() {
  start_span();
  skip_ws();

  if (top_span().empty()) {
    return drop_span();
  }
  else {
    return take_top_span();
  }
}

std::optional<cspan> Parser::take_ws_opt() {
  start_span();
  skip_ws();
  return take_top_span();
}

//------------------------------------------------------------------------------

std::optional<cspan> Parser::take(matcher m) {
  return take_range(cursor, m(cursor));
}

//------------------------------------------------------------------------------

const char* match_lit(const char* text, const char* lit) {
  while(*text && *lit) {
    if (*text != *lit) {
      return nullptr;
    }
    text++;
    lit++;
  }
  return (*lit == 0) ? text : nullptr;
}


std::optional<cspan> Parser::take_lit(const char* lit) {
  return take_range(cursor, match_lit(cursor, lit));
}

std::optional<cspan> Parser::take_lit(const std::vector<const char*>& lits) {
  for(auto lit : lits) {
    if (auto r = take_lit(lit)) return r;
  }
  return std::nullopt;
}

std::optional<cspan> Parser::take_range(const char* begin, const char* end) {
  if (begin && end && begin == cursor) {
    cursor = end;
    return cspan(begin, end);
  }
  else {
    return std::nullopt;
  }
}

//------------------------------------------------------------------------------

PPreprocInclude* Parser::take_preproc_include() {
  start_span();

  auto lit_include = take_lit("#include");
  auto lit_ws      = take(match_ws);
  auto lit_path    = take(match_include_path);

  if (lit_include && lit_ws && lit_path) {
    PPreprocInclude* result = new PPreprocInclude();

    result->lit_include = lit_include.value();
    result->lit_ws      = lit_ws.value();
    result->lit_path    = lit_path.value();

    result->span = take_top_span();
    return result;
  }
  else {
    drop_span();
    return nullptr;
  }
}

//------------------------------------------------------------------------------

// const char* lits[] = {"void", "char", "short", "int", "long", "float", "double" };
// const char* lits[] = { "signed", "unsigned" };
// const char* lits[] = { "auto", "register", "static", "extern", "typedef" };

//------------------------------------------------------------------------------
/*
<enum-specifier> ::= enum <identifier> { <enumerator-list> }
                  | enum { <enumerator-list> }
                  | enum <identifier>
*/

#if 0
std::optional<cspan> Parser::take_enum_specifier() {

  using lit_enum  = Lit<"enum">;
  using enum_list = Seq< Char<'{'>, enum_list, Char<'}'> >;

  using form1 = Seq< lit_enum, identifier, Opt<enum_list> >;
  using form2 = Seq< lit_enum, Opt<identifier>, enum_list >;

  using match = Oneof<form1, form2>;


  start_span();

  // Mandatory enum
  if (!take("enum")) return drop_span();

  // Mandatory whitespace
  if (!take_ws()) return drop_span();

  // Must have either an identifier or an enumerator list;
  bool ok = false;

  // Optional identifier
  if (take_identifier()) {
    take_ws_opt();
    ok = true;
  }

  // Optional enumerator list
  auto bookmark = cursor;
  if (take('{') && take_ws_opt() && take_enumerator_list() && take_ws_opt() && take('}')) {
    ok = true;
  }
  else {
    cursor = bookmark;
  }

  if (ok) {
    return take_top_span();
  }
  else {
    return drop_span();
  }
}
#endif

//------------------------------------------------------------------------------
// <enumerator> ::= <identifier>
//                | <identifier> = <constant-expression>

#if 0
std::optional<cspan> Parser::take_enumerator() {
  start_span();

  if (!take_identifier()) return drop_span();

  if (take('=')) {
    if (!take_constant_expression()) return drop_span();
  }

  return take_top_span();
}
#endif

//------------------------------------------------------------------------------
// <enumerator-list> ::= <enumerator> | <enumerator-list> , <enumerator>

#if 0
std::optional<cspan> Parser::take_enumerator_list() {
  start_span();

  if (!take_enumerator()) return drop_span();

  while(cursor < source_end) {
    take_ws();
    if (!take(',')) break;
    take_ws();
    if (!take_enumerator()) break;
  }

  return take_top_span();
}

//------------------------------------------------------------------------------
// alpha alnum*

std::optional<cspan> Parser::take_identifier() {
  if (auto end = match_identifier(cursor)) {
    return take_span(end);
  }
  return std::nullopt;
}

//------------------------------------------------------------------------------
// type_specifier = primitive_type | signed_specifier | struct_or_union_specifier | enum_specifier | typedef_name

std::optional<cspan> Parser::take_type_specifier() {
  start_span();

  if (take_primitive_type() ||
      take_signed_specifier() ||
      take_struct_or_union_specifier() ||
      take_enum_specifier() ||
      take_typedef_name()) {
    return take_top_span();
  }

  return drop_span();
}

//------------------------------------------------------------------------------
// typedef_name = identifier

std::optional<cspan> Parser::take_typedef_name() {
  return take_identifier();
}

//------------------------------------------------------------------------------
// struct_or_union_specifier =
//   struct_or_union identifier { {struct_declaration}+ } |
//   struct_or_union { {struct_declaration}+ } |
//   struct_or_union identifier

std::optional<cspan> Parser::take_struct_or_union_specifier() {
  start_span();

  if (!take("struct") && !take("union")) return drop_span();

  bool ok = false;

  if (take_identifier()) {
    ok = true;
  }

  if (take_struct_declaration()) {
    ok = true;
  }

  if (ok) {
    return take_top_span();
  }
  else {
    return drop_span();
  }
}

//------------------------------------------------------------------------------
//

std::optional<cspan> Parser::take_constant_expression() {
  assert(false);
  return std::nullopt;
}

//------------------------------------------------------------------------------
// struct_declaration = {specifier_qualifier}* struct_declarator_list

std::optional<cspan> Parser::take_struct_declaration() {
  assert(false);
  return std::nullopt;
}

//------------------------------------------------------------------------------
// specifier_qualifier = type_specifier | type_qualifier

std::optional<cspan> Parser::take_specifier_qualifier() {
  assert(false);
  return std::nullopt;
}

//------------------------------------------------------------------------------
// struct_declarator_list = struct_declarator (, struct_declarator)*

std::optional<cspan> Parser::take_struct_declarator_list() {
  assert(false);
  return std::nullopt;
}

//------------------------------------------------------------------------------
// struct_declarator =
// | declarator
// | declarator : constant_expression
// | : constant_expression

std::optional<cspan> Parser::take_struct_declarator() {
  assert(false);
  return std::nullopt;
}

//------------------------------------------------------------------------------
// declarator = {pointer}? direct_declarator

std::optional<cspan> Parser::take_declarator() {
  assert(false);
  return std::nullopt;
}

//------------------------------------------------------------------------------
// what's with the leading star here?
// pointer = * {type_qualifier}* {pointer}?

std::optional<cspan> Parser::take_pointer() {
  assert(false);
  return std::nullopt;
}

//------------------------------------------------------------------------------
// type-qualifier = const | volatile

std::optional<cspan> Parser::take_type_qualifier() {
  start_span();
  if (take("const") || take("volatile")) {
    return take_top_span();
  }
  else {
    return drop_span();
  }
}
#endif

//------------------------------------------------------------------------------
// translation-unit> = external_declaration*

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

bool Parser::parse_digits(const char* s, int base, uint64_t& out) {
  uint64_t accum = 0;
  int digits = 0;

  while (*s) {
    int digit = 0;
    if (char_to_digit(*s, base, digit)) {
      uint64_t old_accum = accum;
      accum = accum * base + digit;
      if (accum < old_accum) return false;
    }
    else {
      break;
    }
    digits++;
    s++;
  }

  if (isalpha(*s) || digits == 0) {
    return false;
  }

  out = accum;
  return true;
}


bool Parser::parse_int(cspan s, PInt& out) {
  int base = 0;
  uint64_t accum = 0;

  auto cursor = s.begin;

  bool negative = *cursor == '-';
  if (negative) cursor++;

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
    out = PInt(s, negative, 0);
    return true;
  }
  else {
    // bad prefix
    return false;
  }

  if (base && parse_digits(cursor, base, accum)) {
    if (negative && accum > 0x8000000000000000) {
      return false;
    }
    out = PInt(s, negative, accum);
    return true;
  }
  else {
    return false;
  }
}

//------------------------------------------------------------------------------
