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

#include <cstdint>
#include <type_traits>

#include "../../preprocessor.h"
#include "../../utility/integer_sequence.h"
#include "../poly.h"
#include "../sos.h"

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_suppress=Pe540
#endif
// clang-format on

namespace embedded {
namespace signal {
namespace detail {

struct filter_debug_header {
  uint32_t magic{0x544C4946};
  uint16_t length;
  uint16_t filttype;
  uint16_t valtype;
  uint16_t order;
  char name[116];

  template <size_t S>
  constexpr filter_debug_header(uint16_t length, uint8_t filttype,
                                uint8_t valtype, uint8_t order,
                                char const (&name)[S]) noexcept
      : filter_debug_header(
            length, filttype, valtype, order, name,
            make_index_sequence<cmath::min(sizeof(name) - 1, S)>{}) {}

  template <size_t S, std::size_t... Ints>
  constexpr filter_debug_header(uint16_t length_, uint8_t filttype_,
                                uint8_t valtype_, uint8_t order_,
                                char const (&name_)[S],
                                index_sequence<Ints...>) noexcept
      : length{length_}
      , filttype{filttype_}
      , valtype{valtype_}
      , order{order_}
      , name{name_[Ints]...} {}
};

template <typename T>
struct filter_debug_value_type;

template <>
struct filter_debug_value_type<float> {
  static constexpr uint8_t value{0};
};

template <>
struct filter_debug_value_type<double> {
  static constexpr uint8_t value{1};
};

template <typename T>
struct filter_design_debug;

template <typename F, std::size_t N>
struct filter_design_debug<::embedded::signal::sos_design<F, N>> {
  using Design = ::embedded::signal::sos_design<F, N>;
  static constexpr uint16_t size = sizeof(typename Design::sos_array);

  filter_debug_header header;
  typename Design::sos_array coef;

  template <size_t S>
  constexpr filter_design_debug(Design const& d, char const (&name)[S]) noexcept
      : header{size, 0, filter_debug_value_type<F>::value, Design::order(),
               name}
      , coef{d.sos()} {}
};

template <typename F, std::size_t N>
struct filter_design_debug<::embedded::signal::poly_design<F, N>> {
  using Design = ::embedded::signal::poly_design<F, N>;
  static constexpr uint16_t size = 2 * sizeof(typename Design::parray);

  filter_debug_header header;
  typename Design::parray b;
  typename Design::parray a;

  template <size_t S>
  constexpr filter_design_debug(Design const& d, char const (&name)[S]) noexcept
      : header{size, 1, filter_debug_value_type<F>::value, Design::order(),
               name}
      , b{d.b()}
      , a{d.a()} {}
};

} // namespace detail
} // namespace signal
} // namespace embedded

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_default=Pe540
#endif
// clang-format on
