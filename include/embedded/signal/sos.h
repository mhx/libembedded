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

enum class sos_gain {
  first_section,
  distribute,
};

template <typename F>
struct sos_state {
  F y1{}, y2{};
};

template <typename F>
class sos_section {
 public:
  using value_type = F;
  using state_type = sos_state<value_type>;

  template <typename F2>
  constexpr sos_section(cmath::vector<cmath::complex<F2>, 2> const& zeros,
                        cmath::vector<cmath::complex<F2>, 2> const& poles,
                        F2 gain) noexcept
      : sos_section(real(gain * poly(zeros)), real(poly(poles))) {}

  value_type filter(state_type& state, value_type x) const {
    value_type const y = b0_ * x + state.y1;
    state.y1 = b1_ * x - a1_ * y + state.y2;
    state.y2 = b2_ * x - a2_ * y;
    return y;
  }

  constexpr auto b() const noexcept -> cmath::vector<value_type, 3> {
    return cmath::vector<value_type, 3>{b0_, b1_, b2_};
  }

  constexpr auto a() const noexcept -> cmath::vector<value_type, 3> {
    return cmath::vector<value_type, 3>{value_type{1}, a1_, a2_};
  }

 private:
  template <typename F2>
  constexpr sos_section(cmath::vector<F2, 3> const& b,
                        cmath::vector<F2, 3> const& a) noexcept
      : b0_{static_cast<value_type>(b[0])}
      , b1_{static_cast<value_type>(b[1])}
      , b2_{static_cast<value_type>(b[2])}
      , a1_{static_cast<value_type>(a[1])}
      , a2_{static_cast<value_type>(a[2])} {}

  value_type const b0_, b1_, b2_;
  value_type const a1_, a2_;
};

template <typename F, std::size_t N>
class sos_instance;

template <typename F, std::size_t N>
class sos_design {
 public:
  static constexpr std::size_t sos_count{(N + 1) / 2};
  using sos_array = cmath::vector<sos_section<F>, sos_count>;

  template <typename F2>
  static constexpr auto
  build_sos(detail::zpk_value<2 * sos_count, 2 * sos_count, F2> const& zpk,
            sos_gain mode) noexcept -> sos_array {
    return detail::zpk_to_sos<F, F2, sos_count>{}(
        zpk.zeros(), zpk.poles(),
        mode == sos_gain::distribute ? cmath::pow(zpk.gain(), F2{1} / sos_count)
                                     : zpk.gain(),
        mode == sos_gain::distribute);
  }

  template <typename F2>
  constexpr sos_design(detail::zpk_value<N, N, F2> const& zpk,
                       sos_gain mode) noexcept
      : sos_{build_sos(zpk.even(), mode)} {}

  static constexpr std::size_t order() noexcept { return N; }
  static constexpr std::size_t size() noexcept { return sos_count; }

  constexpr auto sos() const noexcept -> sos_array const& { return sos_; }

  constexpr auto instance() const noexcept -> sos_instance<F, N> {
    return sos_instance<F, N>{this};
  }

 private:
  sos_array const sos_;
};

template <typename F, std::size_t N>
class sos_instance {
 public:
  static constexpr std::size_t sos_count{(N + 1) / 2};
  using value_type = F;

  sos_instance(sos_design<value_type, N> const* i) noexcept
      : impl_{i} {}

  value_type operator()(value_type x) {
    return detail::filter_chain<value_type, sos_count>{}(impl_->sos(), state_,
                                                         x);
  }

  auto state() const -> std::array<sos_state<value_type>, sos_count> const& {
    return state_;
  }

 private:
  sos_design<value_type, N> const* impl_;
  std::array<sos_state<value_type>, sos_count> state_{};
};

} // namespace signal
} // namespace embedded

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_default=Pe540
#endif
// clang-format on
