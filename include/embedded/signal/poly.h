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

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_suppress=Pe540
#endif
// clang-format on

namespace embedded {
namespace signal {

template <typename F, std::size_t N>
class poly_instance;

template <typename F, std::size_t N>
class poly_design {
 public:
  using value_type = F;
  using parray = cmath::vector<F, N + 1>;

  template <typename Z>
  constexpr poly_design(Z const& zpk) noexcept
      : b_{real(zpk.gain() * poly(zpk.zeros()))}
      , a_{real(poly(zpk.poles()))} {}

  constexpr std::size_t order() const noexcept { return N; }

  constexpr auto b() const noexcept -> parray { return b_; }
  constexpr auto a() const noexcept -> parray { return a_; }

  F filter(std::array<F, N>& state, F x) const {
    F const y = b_[0] * x + state[0];
    detail::poly_df2t_update<F, N>{}(state, b_, a_, x, y);
    state[N - 1] = b_[N] * x - a_[N] * y;
    return y;
  }

  constexpr auto instance() const noexcept -> poly_instance<F, N> {
    return poly_instance<F, N>(this);
  }

 private:
  parray const b_;
  parray const a_;
};

template <typename F, std::size_t N>
class poly_instance {
 public:
  using value_type = F;

  poly_instance(poly_design<F, N> const* i) noexcept
      : impl_{i} {}

  value_type operator()(value_type x) { return impl_->filter(y_, x); }

 private:
  poly_design<F, N> const* impl_;
  std::array<value_type, N> y_{};
};

} // namespace signal
} // namespace embedded

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_default=Pe540
#endif
// clang-format on
