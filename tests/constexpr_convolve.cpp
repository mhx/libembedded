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

#include "embedded/constexpr_math/convolve.h"
#include "embedded/constexpr_math/vector.h"

#include <gtest/gtest.h>

#include "test_util.h"

using namespace embedded;
using namespace embedded::test;

TEST(constexpr_convolve, basic) {
  constexpr cmath::vector<double, 3> a{1.0, 2.0, 3.0};
  constexpr cmath::vector<double, 3> b{0.0, 1.0, 0.5};
  constexpr auto c = convolve_full(a, b);

  static_assert(c.size() == 5, "size");
  static_assert(c[0] == 0.0, "conv");
  static_assert(almost_equal(c[1], 1.0), "conv");
  static_assert(almost_equal(c[2], 2.5), "conv");
  static_assert(almost_equal(c[3], 4.0), "conv");
  static_assert(almost_equal(c[4], 1.5), "conv");
}
