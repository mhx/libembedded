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

#include "detail/filter.h"

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_suppress=Pe540
#endif
// clang-format on

namespace embedded {
namespace signal {

template <std::size_t Order, typename PF = double>
class chebyshev1 {
 public:
  static_assert(Order > 0, "Filter order must be non-zero");

  static constexpr auto order() noexcept -> std::size_t { return Order; }

  constexpr chebyshev1(PF ripple) noexcept
      : ripple_{ripple} {}

  template <typename F>
  class specification {
   public:
    using value_type = F;

    constexpr specification(PF ripple) noexcept
        : rf_{cmath::sqrt(cmath::pow(PF{10}, PF{0.1} * ripple) - PF{1})} {}

    template <std::size_t N>
    using carray = cmath::vector<cmath::complex<value_type>, N>;

    constexpr auto zeros() const noexcept -> carray<0> { return carray<0>{}; }

    constexpr auto poles() const noexcept -> carray<Order> {
      return (mu() + detail::theta<value_type, Order>()())
          .transform(detail::minus_sinh());
    }

    constexpr auto gain() const noexcept -> value_type {
      return prod(-poles()).real() /
             (Order % 2 == 0 ? cmath::sqrt(value_type{1} + rf_ * rf_)
                             : value_type{1});
    }

    constexpr auto
    zpk() const noexcept -> detail::zpk_value<0, Order, value_type> {
      return detail::zpk_value<0, Order, value_type>(zeros(), poles(), gain());
    }

    constexpr auto rf() const noexcept -> value_type { return rf_; }

   private:
    constexpr value_type mu() const noexcept {
      return value_type{1} / value_type{Order} *
             cmath::asinh(value_type{1} / rf_);
    }

    value_type rf_;
  };

  template <typename F>
  constexpr auto spec() const noexcept -> specification<F> {
    return specification<F>(ripple_);
  }

 private:
  PF const ripple_;
};

template <std::size_t Order, typename PF = double>
class chebyshev2 {
 public:
  static_assert(Order > 0, "Filter order must be non-zero");

  static constexpr auto order() noexcept -> std::size_t { return Order; }

  constexpr chebyshev2(PF ripple) noexcept
      : ripple_{ripple} {}

  template <typename F>
  class specification {
   public:
    using value_type = F;
    static constexpr std::size_t Zn = Order - Order % 2;

    class warp_poles {
     public:
      constexpr warp_poles(value_type mu) noexcept
          : sinh_mu_{cmath::sinh(mu)}
          , cosh_mu_{cmath::cosh(mu)} {}

      constexpr auto
      operator()(cmath::complex<value_type> const& v) const noexcept
          -> cmath::complex<value_type> {
        return cmath::complex<value_type>{1} /
               cmath::complex<value_type>{sinh_mu_ * v.real(),
                                          cosh_mu_ * v.imag()};
      }

     private:
      value_type const sinh_mu_;
      value_type const cosh_mu_;
    };

    constexpr specification(PF ripple) noexcept
        : rf_{PF{1} /
              cmath::sqrt(cmath::pow(PF{10}, PF{0.1} * ripple) - PF{1})} {}

    template <std::size_t N>
    using carray = cmath::vector<cmath::complex<value_type>, N>;

    constexpr auto zeros() const noexcept -> carray<Zn> {
      return value_type{1} /
             detail::theta<value_type, Order, false>()().transform(
                 detail::minus_sinh());
    }

    constexpr auto poles() const noexcept -> carray<Order> {
      return detail::butterworth_poles<value_type, Order>().transform(
          warp_poles(mu()));
    }

    constexpr auto gain() const noexcept -> value_type {
      return (prod(-poles()) / prod(-zeros())).real();
    }

    constexpr auto
    zpk() const noexcept -> detail::zpk_value<Zn, Order, value_type> {
      return detail::zpk_value<Zn, Order, value_type>(zeros(), poles(), gain());
    }

    constexpr auto rf() const noexcept -> value_type { return rf_; }

   private:
    constexpr value_type mu() const noexcept {
      return cmath::asinh(value_type{1} / rf_) / value_type{Order};
    }

    value_type rf_;
  };

  template <typename F>
  constexpr auto spec() const noexcept -> specification<F> {
    return specification<F>(ripple_);
  }

 private:
  PF const ripple_;
};

} // namespace signal
} // namespace embedded

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_default=Pe540
#endif
// clang-format on
