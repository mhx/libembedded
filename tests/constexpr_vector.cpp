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

#include "embedded/constexpr_math/vector.h"

#include <gtest/gtest.h>

using namespace embedded;

namespace {

struct pred_less {
  template <typename T>
  constexpr bool operator()(T const& a, T const& b) const noexcept {
    return a < b;
  }
};

template <typename F>
struct less_than {
  constexpr less_than(F value)
      : value_{value} {}

  constexpr bool operator()(F const& x) const noexcept { return x < value_; }

 private:
  F value_;
};

} // namespace

TEST(constexpr_vector, basic) {
  {
    constexpr cmath::vector<float, 3> a{3.14159f, 2.71f, 1.414f};
    static_assert(a[0] == 3.14159f, "0");
    static_assert(a[1] == 2.71f, "1");
    static_assert(a[2] == 1.414f, "2");
    static_assert(a.argmin(pred_less()) == 2, "argmin");
    static_assert(a.count(less_than<float>{4.0f}) == 3, "count");
    static_assert(a.count(less_than<float>{3.0f}) == 2, "count");
    static_assert(a.count(less_than<float>{2.0f}) == 1, "count");
    static_assert(a.count(less_than<float>{1.0f}) == 0, "count");
  }
  {
    constexpr cmath::vector<int, 5> a1{1, 2, 3, 4, 5};
    constexpr cmath::vector<int, 3> a2{2, 3, 4};
    constexpr cmath::vector<int, 2> a3{1, 5};
    constexpr cmath::vector<int, 2> a4{5, 1};
    constexpr cmath::vector<int, 5> a5{1, 4, 3, 2, 5};
    static_assert(a1.subvector<1, 3>() == a2, "subvector");
    static_assert(a1.argmin(pred_less()) == 0, "argmin");
    static_assert(a1.erase<1, 3>() == a3, "erase");
    static_assert(a3.swap(0, 1) == a4, "swap");
    static_assert(a1.swap(1, 3) == a5, "swap");
  }
  {
    constexpr cmath::vector<int, 4> a{2, 1, 1, 2};
    constexpr cmath::vector<int, 4> sorted{1, 1, 2, 2};
    static_assert(a.argmin(pred_less()) == 1, "argmin");
    static_assert(a.sort(pred_less()) == sorted, "sort");
  }
  {
    constexpr cmath::vector<int, 1> a{2};
    constexpr cmath::vector<int, 1> sorted{2};
    static_assert(a.sort(pred_less()) == sorted, "sort");
    static_assert(cmath::vector<int, 0>{}.sort(pred_less()) ==
                      cmath::vector<int, 0>{},
                  "sort");
  }
  {
    constexpr cmath::vector<int, 12> a{9, 3, 8, 4, 7, 5, 0, 1, 9, 2, 8, 4};
    constexpr cmath::vector<int, 12> sorted{0, 1, 2, 3, 4, 4, 5, 7, 8, 8, 9, 9};
    static_assert(a.sort(pred_less()) == sorted, "sort");
  }
}
