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
#include <ostream>

#include "constexpr_math/complex.h"
#include "constexpr_math/vector.h"
#include "signal/detail/filter.h"

namespace fpm {

template <typename T1, typename T2, unsigned B>
std::ostream& operator<<(std::ostream& os, fixed<T1, T2, B> const& f) {
  os << static_cast<double>(f) << " (" << f.raw_value() << ")";
  return os;
}

} // namespace fpm

namespace embedded {

namespace signal {
namespace detail {

template <std::size_t Zn, std::size_t Pn, typename F>
std::ostream& operator<<(std::ostream& os, zpk_value<Zn, Pn, F> const& zpk) {
  os << "{zeros=" << zpk.zeros() << ", poles=" << zpk.poles()
     << ", gain=" << zpk.gain() << "}";
  return os;
}

template <typename T>
std::ostream&
operator<<(std::ostream& os, embedded::signal::sos_section<T> const& s) {
  os << "{gain=" << s.gain_ << ", b=" << s.b() << ", a=" << s.a() << "}";
  return os;
}

} // namespace detail
} // namespace signal

namespace cmath {

template <typename T>
std::ostream& operator<<(std::ostream& os, complex<T> const& z) {
  os << z.real();
  if (z.imag() >= 0) {
    os << "+";
  }
  os << z.imag() << "j";
  return os;
}

template <typename T, std::size_t Size>
std::ostream& operator<<(std::ostream& os, vector<T, Size> const& a) {
  os << "[";
  for (std::size_t i = 0; i < Size; ++i) {
    if (i > 0) {
      os << ", ";
    }
    os << a[i];
  }
  os << "]";
  return os;
}

} // namespace cmath
} // namespace embedded

namespace std {

template <typename T, std::size_t Size>
std::ostream& operator<<(std::ostream& os, array<T, Size> const& a) {
  os << "[";
  for (std::size_t i = 0; i < Size; ++i) {
    if (i > 0) {
      os << ", ";
    }
    os << a[i];
  }
  os << "]";
  return os;
}

} // namespace std
