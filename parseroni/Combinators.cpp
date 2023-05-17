#include "parseroni/Combinators.h"
#include "metrolib/core/Log.h"

#include <string.h>

using namespace matcheroni;

//------------------------------------------------------------------------------

template<typename M>
struct Tws {
  static const char* match(const char* cursor) {
    cursor = M::match(cursor);
    if(!cursor) return nullptr;
    while(isspace(*cursor)) cursor++;
    return cursor;
  }
};

template<typename M>
struct Lws {
  static const char* match(const char* cursor) {
    if(!cursor) return nullptr;
    while(isspace(*cursor)) cursor++;
    return M::match(cursor);
  }
};

template<typename M>
struct Ws {
  static const char* match(const char* cursor) {
    if(!cursor) return nullptr;
    while(isspace(*cursor)) cursor++;
    cursor = M::match(cursor);
    if(!cursor) return nullptr;
    while(isspace(*cursor)) cursor++;
    return cursor;
  }
};

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

//------------------------------------------------------------------------------

const char* match_punct(const char* text) {
  using trigraphs = Trigraphs<"...<<=>>=">;
  using digraphs  = Digraphs<"---=->!=*=/=&&&=##%=^=+++=<<<===>=>>|=||">;
  using unigraphs = Chars<"-,;:!?.()[]{}*/&#%^+<=>|~">;

  using punct = Oneof<trigraphs, digraphs, unigraphs>;

  return punct::match(text);
}

//------------------------------------------------------------------------------
// Single-line comments

const char* match_oneline_comment(const char* text) {
  using start = Lit<"//">;
  using body  = Any<Seq<Not<Char<'\n'>>, Char<>>>;
  using eol   = Oneof<Char<'\n'>, Char<0>>;

  using line = Seq<start, body, And<eol>>;

  return line::match(text);
}

//------------------------------------------------------------------------------
// Multi-line nested comments

const char* match_multiline_comment(const char* text);
const char* match_comment_body(const char* text);

const char* match_multiline_comment(const char* text) {
  // not-nested version
  using begin = Lit<"/*">;
  using end   = Lit<"*/">;
  using item  = Seq<Not<end>, Char<>>;
  using body  = Seq<begin, Any<item>, end>;
  return body::match(text);

#if 0
  // nested version
  using Begin = Lit<"/*">;
  using End   = Lit<"*/">;
  using Item  = Seq<Not<Begin>, Not<End>, Char<>>;
  using Body  = Ref<match_comment_body>;
  using Delim = Seq<Begin, Body, End>;

  return Delim::match(text);
#endif
}

const char* match_comment_body(const char* text) {
  using Begin = Lit<"/*">;
  using End   = Lit<"*/">;
  using Item  = Seq<Not<Begin>, Not<End>, Char<>>;
  using Delim = Ref<match_multiline_comment>;
  using Body  = Any<Oneof<Delim,Item>>;

  return Body::match(text);
}

//------------------------------------------------------------------------------

const char* match_ws(const char* text) {
  using ws = Char<' ','\t','\r','\n'>;
  return ws::match(text);
}

//------------------------------------------------------------------------------

const char* match_string(const char* text) {
  using quote     = Char<'"'>;
  using item      = Seq<Oneof<Char<'\\'>, Not<quote>>, Char<>>;
  using match_str = Seq<quote, Any<item>, quote>;

  return match_str::match(text);
}

//------------------------------------------------------------------------------

const char* match_raw_string(const char* text) {
  using raw_a     = Lit<"R\"(">;
  using raw_b     = Lit<")\"">;
  using match_raw = Seq<raw_a, Any<Seq<Not<raw_b>, Char<>>>, raw_b>;

  return match_raw::match(text);
}

//------------------------------------------------------------------------------

const char* match_identifier(const char* text) {
  using first = Oneof<Range<'a','z'>, Range<'A','Z'>, Char<'_'>>;
  using rest  = Oneof<Range<'a','z'>, Range<'A','Z'>, Char<'_'>, Range<'0','9'>>;
  using match_id = Seq<first, Any<rest>>;

  return match_id::match(text);
}

//------------------------------------------------------------------------------
// "foo/bar/file.txt"
// <foo/bar/file.txt>

const char* match_include_path(const char* text) {
  using quote     = Char<'"'>;
  using notquote  = Seq<Not<quote>, Char<>>;
  using lbrack    = Char<'<'>;
  using rbrack    = Char<'>'>;
  using notbrack  = Seq<Not<lbrack>, Not<rbrack>, Char<>>;

  using path1 = Seq<quote, Some<notquote>, quote>;
  using path2 = Seq<lbrack, Some<notbrack>, rbrack>;

  using match = Oneof<path1, path2>;

  return match::match(text);
}

//------------------------------------------------------------------------------

const char* match_preproc(const char* text) {
  using chars = Range<'a','z'>;
  using match = Seq< Char<'#'>, Some<chars>, And<Char<' '>> >;
  return match::match(text);
}

//------------------------------------------------------------------------------

const char* match_int(const char* text) {
  using tick       = Char<'\''>;
  using bin_digit  = Oneof<Range<'0','1'>, tick>;
  using oct_digit  = Oneof<Range<'0','7'>, tick>;
  using dec_digit  = Oneof<Range<'0','9'>, tick>;
  using hex_digit  = Oneof<Range<'0','9'>, Range<'a','f'>, Range<'A','F'>, tick>;

  using bin_prefix = Oneof<Lit<"0b">, Lit<"0B">>;
  using oct_prefix = Char<'0'>;
  using dec_prefix = Range<'1','9'>;
  using hex_prefix = Oneof<Lit<"0x">, Lit<"0X">>;

  using match_bin  = Seq<bin_prefix, Some<bin_digit>>;
  using match_oct  = Seq<oct_prefix, Some<oct_digit>>;
  using match_hex  = Seq<hex_prefix, Some<hex_digit>>;
  using match_dec  = Seq<dec_prefix, Any<dec_digit>>;

  using suffix     = Oneof< Lit<"ul">, Lit<"lu">, Lit<"u">, Lit<"l"> >;

  using match_int = Seq<
    Opt<Char<'-'>>,
    Oneof<
      match_bin,
      match_oct,
      match_hex,
      match_dec,
      Char<'0'>
    >,
    Opt<suffix>
  >;

  return match_int::match(text);
}

//------------------------------------------------------------------------------
// floating_constant:
//   | [0-9]*.[0-9]+ ([eE][+-]?[0-9]+)?[flFL]?
//   |        [0-9]+ ([eE][+-]?[0-9]+) [flFL]?

const char* match_float(const char* text) {
  using digit  = Range<'0','9'>;
  using point  = Char<'.'>;
  using sign   = Char<'+','-'>;
  using exp    = Seq<Char<'e','E'>, Opt<sign>, Some<digit>>;
  using suffix = Char<'f','l','F','L'>;

  using with_dot = Seq<Any<digit>, point, Some<digit>, Opt<exp>, Opt<suffix>>;
  using no_dot   = Seq<Some<digit>, exp, Opt<suffix>>;

  using match_float = Oneof<with_dot, no_dot>;

  return match_float::match(text);
}

//------------------------------------------------------------------------------

#if 0
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
#endif


//------------------------------------------------------------------------------
