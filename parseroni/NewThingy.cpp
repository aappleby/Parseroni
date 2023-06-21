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
