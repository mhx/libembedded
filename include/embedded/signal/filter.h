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

#pragma once

#include <array>
#include <cstddef>

#include "detail/filter.h"
#include "poly.h"
#include "sos.h"

// TODO:
// - cleanup
// - return more properties from filter spec (frequency, sample rate, ...)
// - documentation :-)
// - bandpass, bandstop
// - elliptic and bessel filters
// - filter combinations (likely by simply joining poles/zeros)
// - SOS gain optimisation for fixed-point arithmetic

namespace embedded {
namespace signal {

template <typename T = double>
class iirfilter {
 public:
  using value_type = T;

  constexpr iirfilter(value_type fs) noexcept
      : fs_{fs} {}

  template <std::size_t Order>
  class design {
   public:
    using zpk_value = detail::zpk_value<Order, Order, value_type>;

    constexpr design(zpk_value const& zpk) noexcept
        : zpk_{zpk} {}

    template <typename F>
    constexpr auto poly() const noexcept -> poly_design<F, Order> {
      return poly_design<F, Order>(zpk_);
    }

    template <typename F>
    constexpr auto sos(sos_gain mode = sos_gain::first_section) const noexcept
        -> sos_design<F, Order> {
      return sos_design<F, Order>(zpk_, mode);
    }

   private:
    zpk_value const zpk_;
  };

  template <typename C>
  constexpr auto
  lowpass(C const& c, value_type f) const noexcept -> design<C::order()> {
    return design<C::order()>(bilinear_zpk(
        lowpass_zpk(c.template spec<value_type>().zpk(), warp(f))));
  }

  template <typename C>
  constexpr auto
  highpass(C const& c, value_type f) const noexcept -> design<C::order()> {
    return design<C::order()>(bilinear_zpk(
        highpass_zpk(c.template spec<value_type>().zpk(), warp(f))));
  }

 private:
  constexpr value_type warp(value_type f) const noexcept {
    return detail::warp_frequency<value_type>(2.0 * f / fs_, 2.0);
  }

  value_type fs_;
};

} // namespace signal
} // namespace embedded
