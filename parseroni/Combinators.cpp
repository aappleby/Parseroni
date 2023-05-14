#include "parseroni/Combinators.h"
#include "metrolib/core/Log.h"

#include <string.h>

using namespace Parseroni;

//------------------------------------------------------------------------------

const char* match_ws(const char* text) {
  using ws = Char<' ','\t','\r','\n'>;
  return ws::match(text);
}

//------------------------------------------------------------------------------

const char* match_str(const char* text) {
  using quote     = Char<'"'>;
  using notquote  = Seq<Not<quote>, Char<>>;
  using escaped   = Seq<Char<'\\'>, Char<>>;
  using item      = Oneof<escaped, notquote>;
  using match_str = Seq<quote, Any<item>, quote>;

  return match_str::match(text);
}

//------------------------------------------------------------------------------

const char* match_id(const char* text) {
  using first = Oneof<Range<'a','z'>, Range<'A','Z'>, Char<'_'>>;
  using rest  = Oneof<Range<'a','z'>, Range<'A','Z'>, Char<'_'>, Range<'0','9'>>;

  using match_id = Seq<first, Any<rest>>;
  return match_id::match(text);
}

//------------------------------------------------------------------------------

const char* match_int(const char* text) {
  using bin_digit  = Range<'0','1'>;
  using oct_digit  = Range<'0','7'>;
  using dec_digit  = Range<'0','9'>;
  using hex_digit  = Oneof<Range<'0','9'>, Range<'a','f'>, Range<'A','F'>>;

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
int main2() {
  /*
  {
    const char* text = "123.0f tail";
    printf("text  %s\n", text);
    const char* end = match_float(text);
    printf("match %s\n", end);
  }
  */

  /*
  {
    const char* text = "-0x1235459ABCDEFlu tail";
    printf("text  %s\n", text);
    const char* end = match_int(text);
    printf("match %s\n", end);
  }
  */

  {
    const char* text = R"("as\\df\)";
    const char* end1  = text + strlen(text);
    printf("text  %s\n", text);
    const char* end2 = match_str(text);
    printf("match %s\n", end2);
    printf("end1 %p\n", end1);
    printf("end2 %p\n", end2);
    printf("end1 < end2 %d\n", end1 < end2);
  }

  /*
  {
    using matcher = Seq<Some<Char<'a','b'>>, NUL>;
    const char* text = R"(ababababab)";
    printf("text  %s\n", text);
    const char* end = matcher::match(text);
    printf("match %s\n", end);
  }
  */

  return 0;
}
#endif
