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

#include <limits>

#include "embedded/signal/butterworth.h"
#include "embedded/signal/filter.h"

#include <gtest/gtest.h>

#include "test_util.h"

using namespace embedded;
using namespace embedded::signal;
using namespace embedded::test;

namespace {

constexpr double sqrt05 = 0.7071067811865475244;

} // namespace

TEST(signal, butterworth) {
  {
    constexpr auto const bw1p = butterworth<1>().spec<double>().poles();
    static_assert(bw1p.size() == 1, "size");
    static_assert(bw1p[0].real() == -1.0, "real");
    static_assert(bw1p[0].imag() == 0.0, "imag");
    static_assert(butterworth<1>().spec<double>().zeros().size() == 0, "zeros");
  }

  {
    constexpr auto const bw2p = butterworth<2>().spec<double>().poles();
    static_assert(bw2p.size() == 2, "size");
    static_assert(almost_equal(bw2p[0].real(), -sqrt05), "real");
    static_assert(almost_equal(bw2p[0].imag(), sqrt05), "imag");
    static_assert(almost_equal(bw2p[1].real(), -sqrt05), "real");
    static_assert(almost_equal(bw2p[1].imag(), -sqrt05), "imag");
    static_assert(butterworth<2>().spec<double>().zeros().size() == 0, "zeros");
  }

  {
    constexpr auto const bw5p = butterworth<5>().spec<double>().poles();
    static_assert(bw5p.size() == 5, "size");
    static_assert(almost_equal(bw5p[0].real(), -0.30901699437494745), "real");
    static_assert(almost_equal(bw5p[0].imag(), 0.9510565162951535), "imag");
    static_assert(almost_equal(bw5p[1].real(), -0.8090169943749475), "real");
    static_assert(almost_equal(bw5p[1].imag(), 0.5877852522924731), "imag");
    static_assert(bw5p[2].real() == -1.0, "real");
    static_assert(bw5p[2].imag() == 0.0, "imag");
    static_assert(almost_equal(bw5p[3].real(), -0.8090169943749475), "real");
    static_assert(almost_equal(bw5p[3].imag(), -0.5877852522924731), "imag");
    static_assert(almost_equal(bw5p[4].real(), -0.30901699437494745), "real");
    static_assert(almost_equal(bw5p[4].imag(), -0.9510565162951535), "imag");
    static_assert(butterworth<5>().spec<double>().zeros().size() == 0, "zeros");
  }
}

TEST(signal, zpk) {
  constexpr auto zpk = butterworth<2>().spec<double>().zpk();
  static_assert(zpk.poles().size() == 2, "poles");
  static_assert(almost_equal(zpk.poles()[0].real(), -sqrt05), "real");
  static_assert(almost_equal(zpk.poles()[0].imag(), sqrt05), "imag");
  static_assert(almost_equal(zpk.poles()[1].real(), -sqrt05), "real");
  static_assert(almost_equal(zpk.poles()[1].imag(), -sqrt05), "imag");

  constexpr double wn = 2.0 * 100.0 / 1000.0;
  constexpr double fs = 2.0;
  constexpr double expected = 0.9190116821894447;
  constexpr double warped_f = detail::warp_frequency(wn, fs);
  constexpr auto zpk_lp = lowpass_zpk(zpk, warped_f);
  static_assert(almost_equal(warped_f, 1.2996787849316251), "warp");
  static_assert(zpk_lp.poles().size() == 2, "poles");
  static_assert(almost_equal(zpk_lp.poles()[0].real(), -expected), "real");
  static_assert(almost_equal(zpk_lp.poles()[0].imag(), expected), "imag");
  static_assert(almost_equal(zpk_lp.poles()[1].real(), -expected), "real");
  static_assert(almost_equal(zpk_lp.poles()[1].imag(), -expected), "imag");
  static_assert(almost_equal(zpk_lp.gain(), 1.6891649440013454), "gain");

  constexpr auto zpk_bi = bilinear_zpk(zpk_lp, fs);
  static_assert(zpk_bi.zeros().size() == 2, "zeros");
  static_assert(zpk_bi.poles().size() == 2, "poles");
  static_assert(zpk_bi.zeros()[0].real() == -1.0, "real");
  static_assert(zpk_bi.zeros()[0].imag() == 0.0, "imag");
  static_assert(zpk_bi.zeros()[1].real() == -1.0, "real");
  static_assert(zpk_bi.zeros()[1].imag() == 0.0, "imag");
  static_assert(almost_equal(zpk_bi.poles()[0].real(), 0.5714902512699506),
                "real");
  static_assert(almost_equal(zpk_bi.poles()[0].imag(), 0.2935992009519056),
                "imag");
  static_assert(almost_equal(zpk_bi.poles()[1].real(), 0.5714902512699506),
                "real");
  static_assert(almost_equal(zpk_bi.poles()[1].imag(), -0.2935992009519056),
                "imag");
  static_assert(almost_equal(zpk_bi.gain(), 0.06745527388907191), "gain");

  constexpr auto b = zpk_bi.gain() * poly(zpk_bi.zeros());
  constexpr auto a = poly(zpk_bi.poles());
  static_assert(b.size() == 3, "b");
  static_assert(a.size() == 3, "a");
}

TEST(signal, filter) {
  constexpr auto lp =
      iirfilter<double>(1000.0).lowpass(butterworth<2>(), 100.0).poly<double>();

  static_assert(lp.order() == 2, "size");
  static_assert(lp.b().size() == 3, "b.size");
  static_assert(lp.a().size() == 3, "a.size");
  static_assert(almost_equal(lp.b()[0], 0.06745527388907191), "b[0]");
  static_assert(almost_equal(lp.b()[1], 0.13491054777814382), "b[1]");
  static_assert(almost_equal(lp.b()[2], 0.06745527388907191), "b[2]");
  static_assert(almost_equal(lp.a()[0], 1.00000000000000000), "a[0]");
  static_assert(almost_equal(lp.a()[1], -1.1429805025399011), "a[1]");
  static_assert(almost_equal(lp.a()[2], 0.41280159809618866), "a[2]");
}
