#pragma once
#include <algorithm>

const char* match_str(const char* text);
const char* match_id(const char* text);
const char* match_int(const char* text);
const char* match_float(const char* text);

namespace Parseroni {

//------------------------------------------------------------------------------
// Enables using strings as template arguments. Does _not_ include the trailing
// null.

template<size_t N>
struct StringParam {
  constexpr StringParam(const char (&str)[N]) { std::copy_n(str, N-1, value); }
  char value[N-1];
};

//------------------------------------------------------------------------------
// literal character

template <char... rest>
struct Char;

template <char C1, char... rest>
struct Char<C1, rest...> {
  static const char* match(const char* cursor) {
    if (!cursor) return nullptr;
    if (cursor[0] == C1) return ++cursor;
    return Char<rest...>::match(cursor);
  }
};

template <char C1>
struct Char<C1> {
  static const char* match(const char* cursor) {
    if (!cursor) return nullptr;
    if (cursor[0] == C1) return ++cursor;
    return nullptr;
  }
};

template <>
struct Char<> {
  static const char* match(const char* cursor) {
    if (!cursor) return nullptr;
    if (!cursor[0]) return nullptr;
    return ++cursor;
  }
};

//------------------------------------------------------------------------------

struct NUL {
  static const char* match(const char* cursor) {
    if (!cursor) return nullptr;
    if (cursor[0]) return nullptr;
    return cursor;
  }
};

//------------------------------------------------------------------------------
// [chars]

template<StringParam chars>
struct Chars {
  static const char* match(const char* cursor) {
    if(!cursor) return nullptr;
    for (auto i = 0; i < sizeof(chars.value); i++) {
      if (cursor[0] == chars.value[i]) return ++cursor;
    }
    return nullptr;
  }
};

//------------------------------------------------------------------------------
// [A-B]

template<char A, char B>
struct Range {
  static const char* match(const char* cursor) {
    if(!cursor) return nullptr;
    if (cursor[0] >= A && cursor[0] <= B) return cursor + 1;
    return nullptr;
  }
};

//------------------------------------------------------------------------------

template<StringParam lit>
struct Lit {
  static const char* match(const char* cursor) {
    if(!cursor) return nullptr;
    if(!cursor[0]) return nullptr;
    for (auto i = 0; i < sizeof(lit.value); i++) {
      if (cursor[i] == 0) return nullptr;
      if (cursor[i] != lit.value[i]) return nullptr;
    }
    return cursor + sizeof(lit.value);
  }
};

//------------------------------------------------------------------------------
// M?

template<typename M>
struct Opt {
  static const char* match(const char* cursor) {
    if (auto end = M::match(cursor)) return end;
    return cursor;
  }
};

//------------------------------------------------------------------------------
// M*

template<typename M>
struct Any {
  static const char* match(const char* cursor) {
    while(1) {
      auto end = M::match(cursor);
      if (!end) return cursor;
      cursor = end;
    }
    return nullptr;
  }
};

//------------------------------------------------------------------------------
// M+

template<typename M>
struct Some {
  static const char* match(const char* cursor) {
    if (cursor = M::match(cursor)) {
      return Any<M>::match(cursor);
    }
    else {
      return nullptr;
    }
  }
};

//------------------------------------------------------------------------------

template<typename M>
struct Not {
  static const char* match(const char* cursor) {
    return M::match(cursor) ? nullptr : cursor;
  }
};

//------------------------------------------------------------------------------

template<int N, typename M>
struct Rep {
  static const char* match(const char* cursor) {
    for(auto i = 0; i < N; i++) {
      cursor = M::match(cursor);
      if (!cursor) return nullptr;
    }
    return cursor;
  }
};

//------------------------------------------------------------------------------

template <typename M1, typename... rest>
struct Oneof {
  static const char* match(const char* cursor) {
    if (auto end = M1::match(cursor)) return end;
    return Oneof<rest...>::match(cursor);
  }
};

template <typename M1>
struct Oneof<M1> {
  static const char* match(const char* cursor) {
    return M1::match(cursor);
  }
};

//------------------------------------------------------------------------------

template<typename M1, typename... rest>
struct Seq {
  static const char* match(const char* cursor) {
    cursor = M1::match(cursor);
    if (!cursor) return nullptr;
    return Seq<rest...>::match(cursor);
  }
};

template<typename M1>
struct Seq<M1> {
  static const char* match(const char* cursor) {
    return M1::match(cursor);
  }
};

//------------------------------------------------------------------------------

}; // namespace Parseroni
