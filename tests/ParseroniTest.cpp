#include "parseroni/Parser.h"

#include "parseroni/Combinators.h"

#include "metrolib/core/Tests.h"
#include <memory.h>

bool operator == (cspan s, const char* text) {
  for (auto i = 0; i < s.size(); i++) {
    if (text[i] == 0 || s[i] != text[i]) return false;
  }
  return true;
}

//------------------------------------------------------------------------------

TestResults test_basic() {
  TEST_INIT();

#if 0
  Parser p;

  p.load("");
  EXPECT_EQ(false, p.take("asdf").has_value());

  // Test take_until

  p.load("asdf");
  EXPECT_EQ(false, p.take_until('}').has_value());

  p.load("}");
  EXPECT_EQ(true, p.take_until('}').has_value());

  p.load("}");
  EXPECT_EQ(false, p.take_until('}', 1).has_value());

  p.load("a}");
  EXPECT_EQ(true, p.take_until('}', 1).has_value());

  p.load("a}");
  EXPECT_EQ(false, p.take_until('}', 2).has_value());
#endif

  TEST_DONE();
}

//------------------------------------------------------------------------------
//  a+++++b is interpreted as a ++ ++ + b, not as a ++ + ++ b,

TestResults test_tokens() {
  TEST_INIT();

#if 0
  Parser p;
  std::optional<cspan> t;

  p.load("a+++++b");
  t = p.take_token();
  EXPECT_TRUE(t && t.value() == "a");
  t = p.take_token();
  EXPECT_TRUE(t && t.value() == "++");
  t = p.take_token();
  EXPECT_TRUE(t && t.value() == "++");
  t = p.take_token();
  EXPECT_TRUE(t && t.value() == "+");
  t = p.take_token();
  EXPECT_TRUE(t && t.value() == "b");
#endif

  TEST_DONE();
}

//------------------------------------------------------------------------------

TestResults test_match(std::function<const char*(const char*)> m, const char* text) {
  TestResults results;
  auto end = text + strlen(text);
  EXPECT_EQ(end, m(text));
  return results;
}

TestResults test_no_match(std::function<const char*(const char*)> m, const char* text) {
  TestResults results;
  EXPECT_EQ(nullptr, m(text));
  return results;
}

TestResults test_partial_match(std::function<const char*(const char*)> m, const char* text) {
  TestResults results;
  auto end = text + strlen(text);
  auto match = m(text);
  EXPECT_NE(nullptr, match);
  EXPECT_NE(end, match);
  return results;
}

TestResults test_match_int() {
  TEST_INIT();

  results << test_no_match(match_int, "A");
  results << test_no_match(match_int, "");
  results << test_no_match(match_int, "- 1");

  results << test_match(match_int, "0b01010101");
  results << test_match(match_int, "0B01010101");
  results << test_match(match_int, "-0b01010101");
  results << test_match(match_int, "-0B01010101");

  results << test_match(match_int, "01234567");
  results << test_match(match_int, "-01234567");

  results << test_match(match_int, "1234567890");
  results << test_match(match_int, "-1234567890");

  results << test_partial_match(match_int, "01234567890");
  results << test_partial_match(match_int, "-01234567890");

  results << test_match(match_int, "0x0123456789ABCDEF");
  results << test_match(match_int, "0x0123456789abcdef");
  results << test_match(match_int, "0X0123456789ABCDEF");
  results << test_match(match_int, "0X0123456789abcdef");
  results << test_match(match_int, "-0x0123456789ABCDEF");
  results << test_match(match_int, "-0x0123456789abcdef");
  results << test_match(match_int, "-0X0123456789ABCDEF");
  results << test_match(match_int, "-0X0123456789abcdef");

  TEST_DONE();
}

//------------------------------------------------------------------------------
#if 0
TestResults test_take_int() {
  TEST_INIT();

  Parser p;
  std::optional<PInt> x;

  // 0
  p.load("0b0000000000000000000000000000000000000000000000000000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(0ul, x.value().u_int);

  // -0
  p.load("-0b0000000000000000000000000000000000000000000000000000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(true, x.value().is_negative);
  EXPECT_EQ(0ul, x.value().u_int);

  // INT64_MIN
  p.load("-0b1000000000000000000000000000000000000000000000000000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(true, x.value().is_negative);
  EXPECT_EQ(INT64_MIN, x.value().s_int);

  // INT64_MAX
  p.load("0b0111111111111111111111111111111111111111111111111111111111111111");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(INT64_MAX, x.value().s_int);

  // UINT64_MAX
  p.load("0b1111111111111111111111111111111111111111111111111111111111111111");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(UINT64_MAX, x.value().u_int);

  // INT64_MIN - 1
  p.load("-0b1000000000000000000000000000000000000000000000000000000000000001");
  x = p.take_int_as_pint();
  EXPECT_EQ(false, x.has_value());

  // UINT64_MAX + 1
  p.load("0b10000000000000000000000000000000000000000000000000000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(false, x.has_value());

  //----------

  // 0
  p.load("00000000000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(0ul, x.value().u_int);

  // -0
  p.load("-00000000000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(true, x.value().is_negative);
  EXPECT_EQ(0ul, x.value().u_int);

  // INT64_MIN
  p.load("-01000000000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(true, x.value().is_negative);
  EXPECT_EQ(INT64_MIN, x.value().s_int);

  // INT64_MAX
  p.load("00777777777777777777777");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(INT64_MAX, x.value().s_int);

  // UINT64_MAX
  p.load("01777777777777777777777");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(UINT64_MAX, x.value().u_int);

  // INT64_MIN - 1
  p.load("-01000000000000000000001");
  x = p.take_int_as_pint();
  EXPECT_EQ(false, x.has_value());

  // UINT64_MAX + 1
  p.load("02000000000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(false, x.has_value());

  //----------

  // 0
  p.load("0");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(0ul, x.value().u_int);

  // -0
  p.load("-0");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(true, x.value().is_negative);
  EXPECT_EQ(0ul, x.value().u_int);

  // INT64_MIN
  p.load("-9223372036854775808");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(true, x.value().is_negative);
  EXPECT_EQ(INT64_MIN, x.value().s_int);

  // INT64_MAX
  p.load("9223372036854775807");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(INT64_MAX, x.value().s_int);

  // UINT64_MAX
  p.load("18446744073709551615");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(UINT64_MAX, x.value().u_int);

  // INT64_MIN - 1
  p.load("-9223372036854775809");
  x = p.take_int_as_pint();
  EXPECT_EQ(false, x.has_value());

  // UINT64_MAX + 1
  p.load("18446744073709551616");
  x = p.take_int_as_pint();
  EXPECT_EQ(false, x.has_value());


  //----------

  // 0
  p.load("0x0");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(0ul, x.value().u_int);

  // -0
  p.load("-0x0");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(true, x.value().is_negative);
  EXPECT_EQ(0ul, x.value().u_int);

  // INT64_MIN
  p.load("-0x8000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(true, x.value().is_negative);
  EXPECT_EQ(INT64_MIN, x.value().s_int);

  // INT64_MAX
  p.load("0x7FFFFFFFFFFFFFFF");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(INT64_MAX, x.value().s_int);

  // UINT64_MAX
  p.load("0xFFFFFFFFFFFFFFFF");
  x = p.take_int_as_pint();
  EXPECT_EQ(true, x.has_value());
  EXPECT_EQ(false, x.value().is_negative);
  EXPECT_EQ(UINT64_MAX, x.value().u_int);

  // INT64_MIN - 1
  p.load("-0x8000000000000001");
  x = p.take_int_as_pint();
  EXPECT_EQ(false, x.has_value());

  // UINT64_MAX + 1
  p.load("0x10000000000000000");
  x = p.take_int_as_pint();
  EXPECT_EQ(false, x.has_value());

  TEST_DONE();
}
#endif
//------------------------------------------------------------------------------

TestResults test_take_lit() {
  TEST_INIT();

#if 0
  Parser p;

  p.load("asdfasdf");
  auto s1 = p.take("asdf");
  EXPECT_EQ(true, s1.has_value() && s1.value() == "asdf");

  auto s2 = p.take("asdf");
  EXPECT_EQ(true, s2.has_value() && s2.value() == "asdf");

  auto s3 = p.take("asdf");
  EXPECT_EQ(false, s3.has_value());

  // Overrun should not generate a span
  p.load("a");
  EXPECT_EQ(false, p.take("asdf").has_value());

  // Attempting to take an empty string should not generate a span
  EXPECT_EQ(false, p.take("").has_value());
#endif

  TEST_DONE();
}

//------------------------------------------------------------------------------

TestResults test_take_str() {
  TEST_INIT();

#if 0
  Parser p;
  std::optional<cspan> s;

  // simple string
  p.load(R"("asdf")");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("asdf")");

  // trailing stuff after quote
  p.load(R"("asdf"suffix)");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("asdf")");

  // Basic escape sequences should be taken
  p.load(R"("\'\"\?\\\a\b\f\n\r\t\v"suffix)");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\'\"\?\\\a\b\f\n\r\t\v")");

  // 3-digit octal sequences should work but not 2-digit or 4-digit
  p.load(R"("\00"suffix)");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  p.load(R"("\000"suffix)");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\000")");

  p.load(R"("\0000"suffix)");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  // Bracketed octal sequences should work if they contain at least one digit

  p.load(R"("\o{}")suffix")");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  p.load(R"("\o{0}"suffix)");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\o{0}")");

  p.load(R"("\o{7777777777777777777777}"suffix)"); // technically too many bits in constant, but...
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\o{7777777777777777777777}")");

  p.load(R"("\o{080}"suffix)"); // non-octal digit in bracket
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  // Hex sequences should work if they contain at least one digit

  p.load(R"("\xZ"suffix)");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  p.load(R"("\xA"suffix)");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\xA")");

  p.load(R"("\x0123456789ABCDEFZ"suffix)");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\x0123456789ABCDEFZ")");

  // Unicode escape sequences require 4 hex digits

  p.load(R"("\u012")");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  p.load(R"("\u0123")");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\u0123")");

  p.load(R"("\u01234")");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  // Bracketed Unicode escape sequences require at least 1 digit

  p.load(R"("\u{}")");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  p.load(R"("\u{0}")");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\u{0}")");

  p.load(R"("\u{0123456789ABCDEF}")");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\u{0123456789ABCDEF}")");

  p.load(R"("\u{0123456789ABCDEFZ}")");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  // Capital U unicode sequence requires 8 digits

  p.load(R"("\U0123456")");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  p.load(R"("\U01234567")");
  s = p.take_string();
  EXPECT_EQ(true, s && s.value() == R"("\U01234567")");

  p.load(R"("\U012Q4567")");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  p.load(R"("\U012345678")");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  // Named escape sequences require at least one character

  p.load(R"("\N{}")");
  s = p.take_string();
  EXPECT_EQ(false, s.has_value());

  p.load(R"("\N{a}"suffix)");
  s = p.take_string();
  EXPECT_EQ(true, s.has_value() && s.value() == R"("\N{a}")");
#endif

  TEST_DONE();
}

//------------------------------------------------------------------------------

TestResults test_preproc_include() {
  TEST_INIT();

#if 0
  const char* source = R"(#include "some/really/long_path/and/stuff/../stdio.h")";

  Parser p;
  p.load(source);
  auto node = p.take_preproc();

  if (node) {
    LOG_R("Node!\n");
    node.value()->dump();
  }
  else {
    LOG_R("No node?\n");
  }
#endif

  TEST_DONE();
}

//------------------------------------------------------------------------------

TestResults test_hello_world() {
  TEST_INIT("asdf\n");

  /*
  const char* source = R"(
#include <stdio.h>

int main(int argv, char** argc) {
  printf("Hello World!\n");
  return 0;
}
)";
  */

  const char* source = R"(     )";

  Parser p;
  p.load(source);

  auto translation_unit = p.take_translation_unit();
  if (translation_unit) {
    LOG_G("translation_unit:\n");
    translation_unit.value()->dump();
    LOG_G("translation_unit done\n");
  }
  else {
    LOG_R("No translation unit\n");
  }

  TEST_DONE();
}

//------------------------------------------------------------------------------

TestResults test_match_identifier() {
  TEST_INIT();
  TEST_DONE();
}

//------------------------------------------------------------------------------

void print_match(std::function<const char*(const char*)> matcher, const char* text) {
  auto end = text + strlen(text);
  const char* match = matcher(text);
  LOG_CHAR_C('[', 0x00FFFFFF);
  if (match == nullptr) {
    LOG_RANGE_C(text, end, 0x008080FF);
  }
  else {
    LOG_RANGE_C(text, match, 0x0080FF80);
    LOG_RANGE_C(match, end, 0x00404040);
  }
  LOG_CHAR_C(']', 0x00FFFFFF);
  LOG("\n");
}

std::string to_string(std::function<const char*(const char*)> matcher, const char* text) {
  auto end = matcher(text);
  if (end) {
    return std::string(text, end);
  }
  else {
    return "<no match>";
  }
}

TestResults test_match_preproc() {
  TEST_INIT();
  LOG("\n");

  EXPECT_EQ("#include",   to_string(match_preproc, R"(#include <foo/bar/baz.h>)"));
  EXPECT_EQ("<no match>", to_string(match_preproc, R"(#include<foo/bar/baz.h>)"));

  EXPECT_EQ("#include",   to_string(match_preproc, R"(#include "foo/bar/baz.h")"));
  EXPECT_EQ("<no match>", to_string(match_preproc, R"(#include"foo/bar/baz.h")"));

  EXPECT_EQ("<no match>", to_string(match_preproc, R"(include "foo/bar/baz.h")"));

  TEST_DONE();
}

//------------------------------------------------------------------------------

TestResults test_match_path() {
  TEST_INIT();

  results << test_match(match_path, R"("foo/bar/baz.txt")");
  results << test_match(match_path, R"(<foo/bar/baz.txt>)");
  results << test_no_match(match_path, R"(<foo/bar/baz.txt<)");
  results << test_no_match(match_path, R"(>foo/bar/baz.txt>)");

  TEST_DONE();
}

//------------------------------------------------------------------------------

//int main2();

int main(int argc, char** argv) {
  LOG_G("Hello Test World\n");

  //main2();

  TestResults r;
  r << test_basic();

  r << test_match_int();
  //r << test_take_int();
  r << test_take_lit();
  r << test_take_str();
  r << test_tokens();
  r << test_match_path();
  r << test_match_preproc();

  r << test_preproc_include();
  r << test_hello_world();

  return r.show_result();
}

//------------------------------------------------------------------------------
