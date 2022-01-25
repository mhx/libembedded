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

#include <type_traits>

#include "functions.h"

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_suppress=Pe540
#endif
// clang-format on

namespace embedded {
namespace cmath {

template <typename T, std::size_t Size>
class vector;

template <typename T>
class complex {
 public:
  static_assert(std::is_floating_point<T>::value,
                "floating point type required for complex numbers");

  constexpr complex() noexcept = default;

  constexpr complex(T const& real) noexcept
      : real_{real} {}

  constexpr complex(T const& real, T const& imag) noexcept
      : real_{real}
      , imag_{imag} {}

  constexpr T real() const noexcept { return real_; }

  constexpr T imag() const noexcept { return imag_; }

  constexpr T norm() const noexcept { return real_ * real_ + imag_ * imag_; }

  constexpr T abs() const noexcept { return cmath::sqrt(norm()); }

  constexpr T distance(complex<T> const& z) const noexcept {
    return (*this - z).abs();
  }

  constexpr bool is_real() const noexcept { return imag_ == T{}; }

  constexpr auto conj() const noexcept -> complex {
    return complex{real_, -imag_};
  }

  constexpr bool operator==(complex<T> const& z) const noexcept {
    return real_ == z.real_ && imag_ == z.imag_;
  }

 private:
  T real_{}, imag_{};
};

namespace detail {

class real_transformer {
 public:
  template <typename T>
  constexpr auto operator()(cmath::complex<T> const& v) const noexcept -> T {
    return v.real();
  }
};

class imag_transformer {
 public:
  template <typename T>
  constexpr auto operator()(cmath::complex<T> const& v) const noexcept -> T {
    return v.imag();
  }
};

class norm_transformer {
 public:
  template <typename T>
  constexpr auto operator()(cmath::complex<T> const& v) const noexcept -> T {
    return v.norm();
  }
};

class abs_transformer {
 public:
  template <typename T>
  constexpr auto operator()(cmath::complex<T> const& v) const noexcept -> T {
    return v.abs();
  }
};

} // namespace detail

template <typename T>
constexpr auto operator-(complex<T> const& z) noexcept -> complex<T> {
  return complex<T>(-z.real(), -z.imag());
}

template <typename T>
constexpr auto
operator-(complex<T> const& z1, complex<T> const& z2) noexcept -> complex<T> {
  return complex<T>(z1.real() - z2.real(), z1.imag() - z2.imag());
}

template <typename T>
constexpr auto
operator+(complex<T> const& z1, complex<T> const& z2) noexcept -> complex<T> {
  return complex<T>(z1.real() + z2.real(), z1.imag() + z2.imag());
}

template <typename T>
constexpr auto operator-(T v, complex<T> const& z) noexcept -> complex<T> {
  return complex<T>(v) - z;
}

template <typename T>
constexpr auto operator+(T v, complex<T> const& z) noexcept -> complex<T> {
  return complex<T>(v) + z;
}

template <typename T>
constexpr auto
operator*(complex<T> const& z1, complex<T> const& z2) noexcept -> complex<T> {
  return complex<T>(z1.real() * z2.real() - z1.imag() * z2.imag(),
                    z1.real() * z2.imag() + z2.real() * z1.imag());
}

template <typename T>
constexpr auto
operator/(complex<T> const& z1, complex<T> const& z2) noexcept -> complex<T> {
  return (T{1} / z2.norm()) *
         complex<T>(z1.real() * z2.real() + z1.imag() * z2.imag(),
                    z1.imag() * z2.real() - z1.real() * z2.imag());
}

template <typename T>
constexpr auto
operator/(T const& x, complex<T> const& z) noexcept -> complex<T> {
  return complex<T>(x) / z;
}

template <typename T>
constexpr auto
operator/(complex<T> const& z, T const& x) noexcept -> complex<T> {
  return z / complex<T>(x);
}

template <typename T>
constexpr auto
operator*(T const& x, complex<T> const& z) noexcept -> complex<T> {
  return complex<T>(x) * z;
}

template <typename T>
constexpr auto exp(complex<T> const& z) noexcept -> complex<T> {
  return cmath::exp(z.real()) *
         complex<T>(cmath::cos(z.imag()), cmath::sin(z.imag()));
}

template <typename T, std::size_t Size>
constexpr auto
real(vector<complex<T>, Size> const& a) noexcept -> vector<T, Size> {
  return a.template transform<T>(detail::real_transformer());
}

template <typename T, std::size_t Size>
constexpr auto
imag(vector<complex<T>, Size> const& a) noexcept -> vector<T, Size> {
  return a.template transform<T>(detail::imag_transformer());
}

template <typename T, std::size_t Size>
constexpr auto
norm(vector<complex<T>, Size> const& a) noexcept -> vector<T, Size> {
  return a.template transform<T>(detail::norm_transformer());
}

template <typename T, std::size_t Size>
constexpr auto
abs(vector<complex<T>, Size> const& a) noexcept -> vector<T, Size> {
  return a.template transform<T>(detail::abs_transformer());
}

} // namespace cmath
} // namespace embedded

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_default=Pe540
#endif
// clang-format on
