#include "parseroni/Combinators.h"
#include "metrolib/core/Log.h"

#include <string.h>

using namespace parseroni;

const char* origin = nullptr;

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
  using eol   = Oneof<Char<'\n'>, NUL>;

  using line = Seq<start, body, eol>;

  return line::match(text);
}

//------------------------------------------------------------------------------
// Multi-line nested comments

const char* match_multiline_comment(const char* text);
const char* match_comment_body(const char* text);

const char* match_multiline_comment(const char* text) {
  using Begin = Lit<"/*">;
  using End   = Lit<"*/">;
  using Item  = Seq<Not<Begin>, Not<End>, Char<>>;
  using Body  = Ref<match_comment_body>;
  using Delim = Seq<Begin, Body, End>;

  return Delim::match(text);
}

const char* match_comment_body(const char* text) {
  using Begin = Lit<"/*">;
  using End   = Lit<"*/">;
  using Item  = Seq<Not<Begin>, Not<End>, Char<>>;
  using Delim = Ref<match_multiline_comment>;
  using Body  = Any<Oneof<Delim,Item>>;

  return Body::match(text);
}

/*
const char* match_comment_manual(const char* text) {
  if (text[0] != '/') return nullptr;
  if (text[1] != '*') return nullptr;
  text += 2;

  while(text && *text) {
    if (text[0] == '*' && text[1] == '/') {
      return text + 2;
    }
    else if (text[0] == '/' && text[1] == '*') {
      text = match_comment_manual(text);
    }
    else {
      text++;
    }
  }

  return text;
}
*/

//------------------------------------------------------------------------------

const char* match_ws(const char* text) {
  using ws = Char<' ','\t','\r','\n'>;
  return ws::match(text);
}

//------------------------------------------------------------------------------

const char* match_string(const char* text) {
  using quote     = Char<'"'>;
  using notquote  = Seq<Not<quote>, Char<>>;
  using escaped   = Seq<Char<'\\'>, Char<>>;
  using item      = Oneof<escaped, notquote>;
  using match_str = Seq<quote, Any<item>, quote>;

  return match_str::match(text);
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

const char* match_path(const char* text) {
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

const char* match_ab(const char* text) {
  using matcher1 = Some<Char<'a','b'>>;
  return matcher1::match(text);
}

#if 0
int main2() {
  LOG_R("main2\n");

  {
    using trigraphs = Trigraphs<"...<<=>>=">;

    const char* text = ">>=||...<<==asdf";
    printf("text {%s}\n", text);
    text = match_punct(text);
    printf("text {%s}\n", text);
    text = match_punct(text);
    printf("text {%s}\n", text);
    text = match_punct(text);
    printf("text {%s}\n", text);
    text = match_punct(text);
    printf("text {%s}\n", text);
    text = match_punct(text);
    printf("text {%s}\n", text);

  }

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

  /*
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
  */

  /*
  {
    using matcher = Seq<Some<Char<'a','b'>>, NUL>;
    const char* text = R"(ababababab)";
    printf("text  %s\n", text);
    const char* end = matcher::match(text);
    printf("match %s\n", end);
  }
  */

  /*
  {
    using matcher2 = Ref<match_ab>;
    const char* text = R"(abababababtail)";
    printf("text  %s\n", text);
    const char* end = matcher2::match(text);
    printf("match %s\n", end);
  }
  */

#if 0
  {
    volatile const char* text = R"(/*comment comment comment commentcommentcommentcommentcommentcommentcommentcommentcommentcommentcommentcommentcommentcommentcommentcommentcommentcommentcommentcomment comment comment*/ */ tail)";
    origin = (const char*)text;
    printf("text  %s\n", text);
    auto time_a = timestamp();
    size_t accum = 0;
    const int reps = 10000000;
    for (int i = 0; i < reps; i++) {
      const char* end = match_multiline_comment((const char*)text);
      if (end) {
        accum += end - text;
      }
    }
    auto time_b = timestamp();

    //char buf[256] = {0};
    //if (end) memcpy(buf, text, end - text);
    //printf("match {%s}\n", buf);
    //printf("tail  {%s}\n", end);

    printf("accum %ld\n", accum);
    printf("time %f us\n", (time_b - time_a) * 1000000);
    printf("rate %f reps/sec\n",  double(reps) / (time_b - time_a));
    printf("rate %f bytes/sec\n", double(accum) / (time_b - time_a));
  }
#endif

  return 0;
}
#endif
