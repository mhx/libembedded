/*
 * Copyright (c) Marcus Holland-Moritz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "embedded/utility/integer_sequence.h"

#include <gtest/gtest.h>

using namespace embedded;

namespace {

template <typename T1, T1... Ints1, typename T2, T2... Ints2>
constexpr bool equal_integer_sequence(integer_sequence<T1, Ints1...>,
                                      integer_sequence<T2, Ints2...>) noexcept {
  static_assert(std::is_same<integer_sequence<T1, Ints1...>,
                             integer_sequence<T2, Ints2...>>::value,
                "seq");
  return false;
}

} // namespace

TEST(integer_sequence, basic) {
  equal_integer_sequence(make_integer_range<int, 0, 4>{},
                         integer_sequence<int, 0, 1, 2, 3>{});
  equal_integer_sequence(make_integer_range<int, -2, 3>{},
                         integer_sequence<int, -2, -1, 0, 1, 2>{});
  equal_integer_sequence(make_integer_range<int, 2, 5>{},
                         integer_sequence<int, 2, 3, 4>{});
  equal_integer_sequence(make_integer_range<int, 2, 2>{},
                         integer_sequence<int>{});
  equal_integer_sequence(make_integer_range<int, 3, 4>{},
                         integer_sequence<int, 3>{});
  equal_integer_sequence(make_integer_range<int, -3, -2>{},
                         integer_sequence<int, -3>{});
  equal_integer_sequence(make_index_range<2, 4>{},
                         integer_sequence<std::size_t, 2, 3>{});
}
