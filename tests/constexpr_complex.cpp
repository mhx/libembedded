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

#include "embedded/constexpr_math/complex.h"

#include <gtest/gtest.h>

#include "test_util.h"

using namespace embedded;
using namespace embedded::test;

TEST(constexpr_complex, basic) {
  static_assert(cmath::complex<double>(-2.0).is_real(), "is_real");
  static_assert(!cmath::complex<double>(-2.0, -1e-10).is_real(), "is_real");
  static_assert(almost_equal(cmath::complex<double>(-2.0).norm(), 4.0), "norm");
  static_assert(almost_equal(cmath::complex<double>(-2.0).abs(), 2.0), "norm");
  static_assert(almost_equal(cmath::complex<double>(-2.0, 2.0).norm(), 8.0),
                "norm");
  static_assert(cmath::complex<double>(-2.0, 1.0).conj() ==
                    cmath::complex<double>(-2.0, -1.0),
                "conj");
}
