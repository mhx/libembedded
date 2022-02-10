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

enum class filter_debug_structure : uint8_t {
  SOS = 0,
  POLY = 1,
};

enum class filter_debug_value_type : uint8_t {
  FLOAT = 0,
  DOUBLE = 1,
  LONG_DOUBLE = 2,
};

struct filter_debug_header {
  uint32_t magic{0x544C4946};
  uint16_t length;
  uint8_t version{0};
  filter_debug_structure structure;
  filter_debug_value_type valtype;
  char name[119];

  template <size_t S>
  constexpr filter_debug_header(uint16_t length,
                                filter_debug_structure structure,
                                filter_debug_value_type valtype,
                                char const (&name)[S]) noexcept
      : filter_debug_header(
            length, structure, valtype, name,
            make_index_sequence<cmath::min(sizeof(name) - 1, S)>{}) {}

  template <size_t S, std::size_t... Ints>
  constexpr filter_debug_header(uint16_t length_,
                                filter_debug_structure structure_,
                                filter_debug_value_type valtype_,
                                char const (&name_)[S],
                                index_sequence<Ints...>) noexcept
      : length{length_}
      , structure{structure_}
      , valtype{valtype_}
      , name{name_[Ints]...} {}
};

static_assert(sizeof(filter_debug_header) % 8 == 0,
              "unexpected filter_debug_header size");

template <typename T>
struct filter_debug_value_type_of;

template <>
struct filter_debug_value_type_of<float> {
  static constexpr auto value{filter_debug_value_type::FLOAT};
};

template <>
struct filter_debug_value_type_of<double> {
  static constexpr auto value{filter_debug_value_type::DOUBLE};
};

template <>
struct filter_debug_value_type_of<long double> {
  static constexpr auto value{filter_debug_value_type::LONG_DOUBLE};
};

template <typename T>
struct filter_design_debug;

template <typename F, std::size_t N>
struct filter_design_debug<::embedded::signal::sos_design<F, N>> {
  using Design = ::embedded::signal::sos_design<F, N>;
  static constexpr uint16_t size =
      sizeof(filter_debug_header) + sizeof(typename Design::sos_array);

  filter_debug_header header;
  typename Design::sos_array coef;

  template <size_t S>
  constexpr filter_design_debug(Design const& d, char const (&name)[S]) noexcept
      : header{size, filter_debug_structure::SOS,
               filter_debug_value_type_of<F>::value, name}
      , coef{d.sos()} {}
};

template <typename F, std::size_t N>
struct filter_design_debug<::embedded::signal::poly_design<F, N>> {
  using Design = ::embedded::signal::poly_design<F, N>;
  static constexpr uint16_t size =
      sizeof(filter_debug_header) + 2 * sizeof(typename Design::parray);

  filter_debug_header header;
  typename Design::parray b;
  typename Design::parray a;

  template <size_t S>
  constexpr filter_design_debug(Design const& d, char const (&name)[S]) noexcept
      : header{size, filter_debug_structure::POLY,
               filter_debug_value_type_of<F>::value, name}
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
