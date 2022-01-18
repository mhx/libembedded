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

#include "convolve.h"

namespace embedded {
namespace cmath {

namespace detail {

template <std::size_t N>
struct poly_rec {
  template <typename T, std::size_t S1, std::size_t S2>
  constexpr auto
  operator()(vector<T, S1> const& a, vector<T, S2> const& zeros) const noexcept
      -> vector<T, S1 + N> {
    return convolve_full(poly_rec<N - 1>()(a, zeros),
                         vector<T, 2>{T{1}, -zeros[N - 1]});
  }
};

template <>
struct poly_rec<0> {
  template <typename T, std::size_t S1, std::size_t S2>
  constexpr auto
  operator()(vector<T, S1> const& a,
             vector<T, S2> const& /*zeros*/) const noexcept -> vector<T, S1> {
    return a;
  }
};

} // namespace detail

template <typename T, std::size_t S>
constexpr auto poly(vector<T, S> const& zeros) noexcept -> vector<T, S + 1> {
  return detail::poly_rec<S>()(vector<T, 1>{T{1}}, zeros);
}

} // namespace cmath
} // namespace embedded
