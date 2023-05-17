#include <stdio.h>

#include <string.h>
#include <filesystem>
#include <regex>

#include "metrolib/core/Tests.h"
#include "parseroni/Combinators.h"

using namespace matcheroni;

using rdit = std::filesystem::recursive_directory_iterator;

enum SourceTag {
  // Lexical elements
  KEYWORD,
  IDENTIFIER,
  CONSTANT,
  STRING,
  PUNCTUATOR,
  PREPROC,

  // Delimited blocks
  BLOCK_CURLY,
  BLOCK_SQUARE,
  BLOCK_ANGLE,
  BLOCK_PAREN,
};

struct SourceMark {
  SourceTag tag;
  int end;
};

const char* match_space(const char* text) {
  using ws = Char<' ','\t'>;
  return ws::match(text);
}

const char* match_newline(const char* text) {
  //using nl = Oneof<Lit<"\r\n">, Char<'\r'>, Char<'\n'>>;
  using nl = Some<Oneof<Char<'\r'>, Char<'\n'>>>;
  return nl::match(text);
}

const char* match_escape(const char* text) {
  using octal_digit   = Range<'0','7'>;
  using escaped_octal = Seq<Char<'\\'>, octal_digit, octal_digit, octal_digit>;
  using escaped_char  = Seq<Char<'\\'>, Char<'0', 'a', 'b', 'f', 'n', 'r', 'v', 't', '\\', '\''>>;

  using escaped = Oneof<escaped_octal, escaped_char>;

  return escaped::match(text);
}

const char* match_char_literal(const char* text) {
  using tick = Char<'\''>;
  using escaped = Ref<match_escape>;
  using unescaped = Char<>;
  using match = Seq<tick, Oneof<escaped, unescaped>, tick>;
  return match::match(text);
}

void log_span(const char* start, const char* end, uint32_t color = 0) {
  auto& log = TinyLog::get();

  log.color(color);
  log.write(start, end - start);

  /*
  for (auto cursor = start; cursor < end; cursor++) {
    auto c = *cursor;
    if (c == '\t') {
    }
    else if (c == '\n') {
    }
    else if (c == '\r') {
    }
    else if (c == ' ') {
    }
    else {
    }
  }
  */
  /*
  LOG_CHAR_C(' ', 0x808080);
  LOG_CHAR_C('{', 0x808080);
  //fwrite(cursor, 1, end - cursor, stdout);
  LOG_RANGE_C(start, end, 0x80FF80);
  LOG_CHAR_C('}', 0x808080);
  LOG_CHAR_C('\n', 0x808080);
  */
}

int hit_char_literal = 0;
int hit_comment1 = 0;
int hit_comment2 = 0;
int hit_float = 0;
int hit_identifier = 0;
int hit_int = 0;
int hit_newline = 0;
int hit_preproc = 0;
int hit_punct = 0;
int hit_raw_string = 0;
int hit_space = 0;
int hit_splice = 0;
int hit_string = 0;

//------------------------------------------------------------------------------

TestResults test_scan(const std::string path, size_t size) {
  TestResults results;

  std::string text;
  text.resize(size);
  FILE* f = fopen(path.c_str(), "rb");
  auto r = fread(text.data(), 1, size, f);
  fclose(f);

  const char* cursor = text.data();

  while(*cursor) {
    /*
    if (auto end = Lit<"\n\r">::match(cursor)) {
      printf("weird newline\n");
      cursor = end;
    }
    else
    */

    //log_span(cursor, end);

    // Lines ending in a backslash and a newline get spliced together with the following line
    if (auto end = Lit<"\\\n">::match(cursor)) {
      hit_splice++;
      cursor = end;
    }
    if (auto end = match_preproc(cursor)) {
      hit_preproc++;
      cursor = end;
    }
    else if (auto end = match_raw_string(cursor)) {
      hit_raw_string++;
      cursor = end;
    }
    else if (auto end = match_float(cursor)) {
      hit_float++;
      cursor = end;
    }
    else if (auto end = match_space(cursor)) {
      hit_space++;
      cursor = end;
    }
    else if (auto end = match_newline(cursor)) {
      hit_newline++;
      cursor = end;
    }
    else if (auto end = match_string(cursor)) {
      hit_string++;
      cursor = end;
    }
    else if (auto end = match_oneline_comment(cursor)) {
      hit_comment1++;
      cursor = end;
    }
    else if (auto end = match_multiline_comment(cursor)) {
      hit_comment2++;
      cursor = end;
    }
    else if (auto end = match_identifier(cursor)) {
      hit_identifier++;
      cursor = end;
    }
    else if (auto end = match_int(cursor)) {
      hit_int++;
      cursor = end;
    }
    else if (auto end = match_char_literal(cursor)) {
      hit_char_literal++;
      cursor = end;
    }
    else if (auto end = match_punct(cursor)) {
      hit_punct++;
      cursor = end;
    }
    else {
      //LOG_R("File %s:\nCould not match {%.40s}\n\n", path.c_str(), cursor);
      results.test_fail++;
      return results;
    }
  }

  results.test_pass++;
  return results;
}

//------------------------------------------------------------------------------

int total_files = 0;
int total_bytes = 0;
int source_files = 0;

TestResults test_dir(const char* base_path) {
  TestResults results;
  for (const auto& f : rdit(base_path)) {
    //printf("path %s\n", f.path().c_str());
    //printf("size %d\n", f.file_size());
    if (f.is_regular_file()) {
      total_files++;
      auto& path = f.path().native();
      auto size = f.file_size();
      if (path.ends_with(".h") || path.ends_with(".cpp") || path.ends_with(".c")) {
        source_files++;
        results << test_scan(path, size);
        total_bytes += size;
      }
    }
  }
  return results;
}

//------------------------------------------------------------------------------

void benchmark() {
  const int buf_size = 1000000;
  std::string buf;
  buf.resize(buf_size + 1);

  const char* text = "()abcdef";
  auto text_len = strlen(text);

  for (int i = 0; i < buf_size; i++) {
    buf[i] = text[rand() % text_len];
  }
  buf.back() = 0;
  //printf("%s\n", buf.c_str());

  for (int rep = 0; rep < 10; rep++) {
    double r_time_a, r_time_b;
    double m_time_a, m_time_b;
    double c_time_a, c_time_b;

    {
      //std::regex paren_match_regex(R"(\(\w+\))");
      std::regex paren_match_regex(R"(\([^()]+\))");
      //std::regex paren_match_regex(R"(\([^)]+\))");

      int matches = 0;
      r_time_a = timestamp();
      const char* cursor = buf.c_str();
      while(cursor && cursor[0]) {
        std::cmatch m;
        if (std::regex_search(cursor, m, paren_match_regex)) {
          matches++;
          auto p = m.position();
          auto l = m.length();
          //printf("position %d\n", p);
          //printf("length   %d\n", l);
          //log_span(cursor + p, cursor + p + l, 0x00FFFF);
          //LOG("\n");
          cursor = cursor + p + l;
          //cursor += m[0].length();
        }
        else {
          cursor++;
        }
      }
      r_time_b = timestamp();
      printf("std::regex match count %d\n", matches);
      printf("std::regex elapsed time %f ms\n", (r_time_b - r_time_a) * 1000.0);
    }

    {
      using lparen = Char<'('>;
      using rparen = Char<')'>;
      using item   = Seq<Not<lparen>, Not<rparen>, Char<>>;
      //using item = NChar<'(', ')'>;
      using match  = Seq<lparen, Some<item>, rparen>;

      int matches = 0;
      m_time_a = timestamp();
      const char* cursor = buf.c_str();
      while(cursor && cursor[0]) {
        if (auto end = match::match(cursor)) {
          matches++;
          cursor = end;
        }
        else {
          cursor++;
        }
      }
      m_time_b = timestamp();
      printf("Matcheroni match count %d\n", matches);
      printf("Matcheroni elapsed time %f ms\n", (m_time_b - m_time_a) * 1000.0);
    }

    {
      int matches = 0;
      c_time_a = timestamp();
      const char* cursor = buf.c_str();
      const char* mark = 0;

      while(cursor && cursor[0]) {
        if (*cursor == '(') {
          mark = cursor;
        } else if (*cursor == ')') {
          if (mark && ((cursor - mark) > 1)) matches++;
          mark = nullptr;
        }
        cursor++;
      }

      c_time_b = timestamp();
      printf("Manual match count %d\n", matches);
      printf("Manual elapsed time %f ms\n", (c_time_b - c_time_a) * 1000.0);
    }

    printf("Matcheroni is %f times faster than std::regex\n", (r_time_b - r_time_a) / (m_time_b - m_time_a));
    printf("Matcheroni is %f times faster than manual\n", (c_time_b - c_time_a) / (m_time_b - m_time_a));
  }
}

//------------------------------------------------------------------------------

TestResults test_thingy() {
  TEST_INIT();

  //const char* text = "'\\002'";
  //const char* end = match_escape(text);
  //printf("text {%s}\n", text);
  //printf("end  {%s}\n", end);

  const char* text = "hello world\n";
  log_span(text, text + strlen(text), 0);

  benchmark();

#if 0
  results << test_dir("../MetroLib");
  results << test_dir("../Metron");
  results << test_dir(".");
  results << test_dir("../gcc/gcc");

  printf("total files %d\n", total_files);
  printf("source files %d\n", source_files);
  printf("total bytes %d\n", total_bytes);

  printf("hit_char_literal %d\n", hit_char_literal);
  printf("hit_comment1     %d\n", hit_comment1    );
  printf("hit_comment2     %d\n", hit_comment2    );
  printf("hit_float        %d\n", hit_float       );
  printf("hit_identifier   %d\n", hit_identifier  );
  printf("hit_int          %d\n", hit_int         );
  printf("hit_newline      %d\n", hit_newline     );
  printf("hit_preproc      %d\n", hit_preproc     );
  printf("hit_punct        %d\n", hit_punct       );
  printf("hit_raw_string   %d\n", hit_raw_string  );
  printf("hit_space        %d\n", hit_space       );
  printf("hit_splice       %d\n", hit_splice      );
  printf("hit_string       %d\n", hit_string      );
#endif

  TEST_DONE();
}

//------------------------------------------------------------------------------
