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

#include <cstddef>

#include "detail/bessel_poles.h"
#include "detail/filter.h"

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_suppress=Pe540
#endif
// clang-format on

namespace embedded {
namespace signal {

template <std::size_t Order>
class bessel {
 public:
  static_assert(Order > 0, "Filter order must be non-zero");

  static constexpr auto order() noexcept -> std::size_t { return Order; }

  constexpr bessel() noexcept = default;

  template <typename F>
  class specification {
   public:
    using value_type = F;

    template <std::size_t N>
    using carray = cmath::vector<cmath::complex<value_type>, N>;

    constexpr auto zeros() const noexcept -> carray<0> { return carray<0>{}; }

    constexpr auto poles() const noexcept -> carray<Order> {
      return detail::bessel_poles<value_type, Order>::value;
    }

    constexpr auto gain() const noexcept -> value_type { return value_type{1}; }

    constexpr auto
    zpk() const noexcept -> detail::zpk_value<0, Order, value_type> {
      return detail::zpk_value<0, Order, value_type>(zeros(), poles(), gain());
    }
  };

  template <typename F>
  constexpr auto spec() const noexcept -> specification<F> {
    return specification<F>();
  }
};

} // namespace signal
} // namespace embedded

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_default=Pe540
#endif
// clang-format on
