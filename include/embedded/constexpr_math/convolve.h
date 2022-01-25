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

#include "../utility/integer_sequence.h"
#include "vector.h"

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_suppress=Pe540
#endif
// clang-format on

namespace embedded {
namespace cmath {

namespace detail {

template <typename T, std::size_t S1, std::size_t S2, typename I>
constexpr auto convolve_single(vector<T, S1> const& a, vector<T, S2> const& b,
                               I n, I m = 0) noexcept -> T {
  return (m >= 0 && m < I(a.size()) ? a[m] : T{}) *
             (n - m >= 0 && n - m < I(b.size()) ? b[n - m] : T{}) +
         (m < n ? convolve_single(a, b, n, m + 1) : T{});
}

template <typename T, std::size_t S1, std::size_t S2, std::size_t... Ints>
constexpr auto
convolve_full_impl(vector<T, S1> const& a, vector<T, S2> const& b,
                   index_sequence<Ints...>) noexcept -> vector<T, S1 + S2 - 1> {
  return vector<T, S1 + S2 - 1>{convolve_single(a, b, Ints)...};
}

} // namespace detail

template <typename T, std::size_t S1, std::size_t S2>
constexpr auto
convolve_full(vector<T, S1> const& a, vector<T, S2> const& b) noexcept
    -> vector<T, S1 + S2 - 1> {
  return detail::convolve_full_impl(a, b, make_index_sequence<S1 + S2 - 1>{});
}

} // namespace cmath
} // namespace embedded

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_default=Pe540
#endif
// clang-format on
