#pragma once
#include <algorithm>
#include <assert.h>
#include <functional>

#include "symlinks/Matcheroni/Matcheroni.h"

using matcher = std::function<const char*(const char*)>;

struct cspan {
  cspan() : begin(nullptr), end(nullptr) {
  }

  cspan(const char* begin, const char* end) {
    if (!begin || !end) {
      this->begin = nullptr;
      this->end = nullptr;
    }
    else {
      this->begin = begin;
      this->end = end;
    }
  }

  bool operator == (const char* text) const {
    int i = 0;
    for (; i < size(); i++) {
      if (begin[i] != text[i]) return false;
    }
    return text[i] == 0;
  }

  char operator[](size_t i) const {
    assert(begin);
    assert(i < size());
    return begin[i];
  }

  size_t size() const { return end - begin; }
  bool empty() const { return end == begin; }

  cspan operator + (const cspan& b) const {
    if (end == b.begin) {
      return cspan(begin, b.end);
    }
    else {
      return cspan(nullptr, nullptr);
    }
  }

  const char* begin;
  const char* end;
};


const char* match_space(const char* text);
const char* match_newline(const char* text);
const char* match_char_literal(const char* text);
const char* match_string(const char* text);
const char* match_identifier(const char* text);
const char* match_int(const char* text);
const char* match_float(const char* text);
const char* match_punct(const char* text);
const char* match_preproc(const char* text);
const char* match_include_path(const char* text);
const char* match_lit(const char* text, const char* lit);
const char* match_ws(const char* text);
const char* match_raw_string(const char* text);
const char* match_oneline_comment(const char* text);
const char* match_multiline_comment(const char* text);
