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

#include "../../constexpr_math.h"
#include "../../utility/integer_sequence.h"

namespace embedded {
namespace signal {

template <typename F>
struct sos_state;

template <typename F>
class sos_section;

namespace detail {

template <std::size_t Zn, std::size_t Pn, typename F>
class zpk_value {
 public:
  using value_type = F;

  template <std::size_t N>
  using carray = cmath::vector<cmath::complex<value_type>, N>;

  constexpr zpk_value(carray<Zn> const& z, carray<Pn> const& p, value_type k)
      : z_{z}
      , p_{p}
      , k_{k} {}

  constexpr auto zeros() const noexcept -> carray<Zn> const& { return z_; }
  constexpr auto poles() const noexcept -> carray<Pn> const& { return p_; }
  constexpr auto gain() const noexcept -> value_type { return k_; }

  constexpr auto
  even() const noexcept -> zpk_value<Zn + Zn % 2, Pn + Pn % 2, F> {
    return zpk_value<Zn + Zn % 2, Pn + Pn % 2, F>{
        z_.append(carray<Zn % 2>::zeros()), p_.append(carray<Pn % 2>::zeros()),
        k_};
  }

 private:
  carray<Zn> const z_;
  carray<Pn> const p_;
  value_type const k_;
};

template <typename F, std::size_t N, bool IncludeZero = true>
class theta {
 public:
  static constexpr bool RemoveZero = !IncludeZero && N % 2 != 0;
  static constexpr std::size_t M = N - static_cast<std::size_t>(RemoveZero);

  constexpr auto
  operator()() const noexcept -> cmath::vector<cmath::complex<F>, M> {
    return cmath::make_vector<cmath::complex<F>>(single_pole,
                                                 make_index_sequence<M>{});
  }

 private:
  static constexpr int index(std::size_t i) noexcept {
    return 2 * (int(i) + int(RemoveZero && int(i) >= int(N) / 2)) + 1 - int(N);
  }

  static constexpr auto
  single_pole(std::size_t i) noexcept -> cmath::complex<F> {
    return cmath::complex<F>(F{0}, F{cmath::pi<F>() * index(i) / (2 * int(N))});
  }
};

struct minus_sinh {
  template <typename F>
  constexpr auto
  operator()(cmath::complex<F> const& v) const noexcept -> cmath::complex<F> {
    return -(exp(v) - exp(-v)) / F{2};
  }
};

template <typename F>
constexpr auto warp_frequency(F freq, F fs) noexcept -> F {
  return F{2} * fs * cmath::tan(cmath::pi<F>() * freq / fs);
}

template <typename F, std::size_t Zn, std::size_t Pn>
constexpr auto lowpass_zpk(zpk_value<Zn, Pn, F> const& zpk, F f) noexcept
    -> zpk_value<Zn, Pn, F> {
  return zpk_value<Zn, Pn, F>(f * zpk.zeros(), f * zpk.poles(),
                              zpk.gain() * cmath::pow(f, Pn - Zn));
}

template <typename F>
class highpass_gain {
 public:
  constexpr auto
  operator()(cmath::complex<F> const& a,
             cmath::complex<F> const& v) const noexcept -> cmath::complex<F> {
    return a * (-v);
  }
};

template <typename F, std::size_t Zn, std::size_t Pn>
constexpr auto highpass_zpk(zpk_value<Zn, Pn, F> const& zpk, F f) noexcept
    -> zpk_value<Pn, Pn, F> {
  return zpk_value<Pn, Pn, F>(
      (f / zpk.zeros())
          .append(cmath::vector<cmath::complex<F>, Pn - Zn>::zeros()),
      f / zpk.poles(),
      zpk.gain() * (zpk.zeros().reduce(highpass_gain<F>{}) /
                    zpk.poles().reduce(highpass_gain<F>{}))
                       .real());
}

template <typename F>
class bilinear_zp {
 public:
  constexpr bilinear_zp(F fs)
      : fs2_{F{2} * fs} {}

  constexpr auto
  operator()(cmath::complex<F> const& v) const noexcept -> cmath::complex<F> {
    return (fs2_ + v) / (fs2_ - v);
  }

 private:
  F fs2_;
};

template <typename F>
class bilinear_gain {
 public:
  constexpr bilinear_gain(F fs)
      : fs2_{F{2} * fs} {}

  constexpr auto
  operator()(cmath::complex<F> const& a,
             cmath::complex<F> const& v) const noexcept -> cmath::complex<F> {
    return a * (fs2_ - v);
  }

 private:
  F fs2_;
};

template <typename F, std::size_t Zn, std::size_t Pn>
constexpr auto
bilinear_zpk(zpk_value<Zn, Pn, F> const& zpk, F fs = F{2}) noexcept
    -> zpk_value<Pn, Pn, F> {
  return zpk_value<Pn, Pn, F>(
      zpk.zeros()
          .transform(bilinear_zp<F>{fs})
          .append(cmath::vector<cmath::complex<F>, Pn - Zn>::full(-1)),
      zpk.poles().transform(bilinear_zp<F>{fs}),
      zpk.gain() * (zpk.zeros().reduce(bilinear_gain<F>{fs}) /
                    zpk.poles().reduce(bilinear_gain<F>{fs}))
                       .real());
}

template <typename F, std::size_t N, std::size_t C = N - 1>
class poly_df2t_update {
 public:
  using parray = cmath::vector<F, N + 1>;
  static constexpr std::size_t I = N - C - 1;

  void operator()(std::array<F, N>& state, parray const& b, parray const& a,
                  F x, F y) const {
    state[I] = b[I + 1] * x - a[I + 1] * y + state[I + 1],
    poly_df2t_update<F, N, C - 1>{}(state, b, a, x, y);
  }
};

template <typename F, std::size_t N>
class poly_df2t_update<F, N, 0> {
 public:
  using parray = cmath::vector<F, N + 1>;

  void operator()(std::array<F, N>&, parray const&, parray const&, F, F) const {
  }
};

class is_real {
 public:
  template <typename F>
  constexpr bool operator()(cmath::complex<F> const& x) const noexcept {
    return x.is_real();
  }
};

template <typename F>
static constexpr F unit_distance(cmath::complex<F> const& z) noexcept {
  // norm() is cheaper than abs()
  return cmath::abs(F{1} - z.norm());
}

struct unit_circle_distance_less {
 public:
  template <typename F>
  constexpr bool operator()(cmath::complex<F> const& a,
                            cmath::complex<F> const& b) const noexcept {
    return unit_distance(a) < unit_distance(b);
  }
};

struct unit_circle_distance_real_less {
 public:
  template <typename F>
  constexpr bool operator()(cmath::complex<F> const& a,
                            cmath::complex<F> const& b) const noexcept {
    return a.is_real() < b.is_real() ||
           (a.is_real() == a.is_real() && unit_distance(a) < unit_distance(b));
  }
};

template <typename F>
class distance_less {
 public:
  constexpr distance_less(cmath::complex<F> const& z) noexcept
      : z_{z} {}

  constexpr bool operator()(cmath::complex<F> const& a,
                            cmath::complex<F> const& b) const noexcept {
    return (a - z_).norm() < (b - z_).norm();
  }

 private:
  cmath::complex<F> const z_;
};

template <typename F>
class distance_real_less {
 public:
  constexpr distance_real_less(cmath::complex<F> const& z) noexcept
      : z_{z} {}

  constexpr bool operator()(cmath::complex<F> const& a,
                            cmath::complex<F> const& b) const noexcept {
    return a.is_real() > b.is_real() ||
           (a.is_real() == b.is_real() && (a - z_).norm() < (b - z_).norm());
  }

 private:
  cmath::complex<F> const z_;
};

template <typename F>
class distance_complex_less {
 public:
  constexpr distance_complex_less(cmath::complex<F> const& z) noexcept
      : z_{z} {}

  constexpr bool operator()(cmath::complex<F> const& a,
                            cmath::complex<F> const& b) const noexcept {
    return a.is_real() < b.is_real() ||
           (a.is_real() == b.is_real() && (a - z_).norm() < (b - z_).norm());
  }

 private:
  cmath::complex<F> const z_;
};

template <typename FO, typename F, std::size_t Stages>
class zpk_to_sos {
 public:
  using value_type = F;
  using output_type = FO;

  using cnum = cmath::complex<value_type>;

  template <std::size_t N>
  using carray = cmath::vector<cnum, N>;

  template <std::size_t N>
  using sos_array = cmath::vector<sos_section<output_type>, N>;

  constexpr auto
  operator()(carray<2 * Stages> const& z, carray<2 * Stages> const& p,
             value_type gain, bool distribute_gain) const noexcept
      -> sos_array<Stages> {
    return step0(next_pole_index(p), z, p, gain, distribute_gain);
  }

 private:
  constexpr auto
  step0(std::size_t p1, carray<2 * Stages> const& z,
        carray<2 * Stages> const& p, value_type gain,
        bool distribute_gain) const noexcept -> sos_array<Stages> {
    return step1(p[p1], z, p.swappop(p1), gain, distribute_gain);
  }

  constexpr auto
  step1(cnum p1, carray<2 * Stages> const& z, carray<2 * Stages - 1> const& p,
        value_type gain, bool distribute_gain) const noexcept
      -> sos_array<Stages> {
    return !p1.is_real() && z.count(is_real{}) == 1 && p.count(is_real{}) == 1
               ? step2a(nearest_complex_index(z, p1), p1, z, p, gain,
                        distribute_gain)
               : step2b(p1,
                        p1.is_real() ? next_real_pole_index(p)
                                     : conjugate_index(p, p1),
                        z, p, gain, distribute_gain);
  }

  // We have a complex zero and a complex pole. We need to find and add
  // the complex conjugates.
  constexpr auto
  step2a(std::size_t z1, cnum p1, carray<2 * Stages> const& z,
         carray<2 * Stages - 1> const& p, value_type gain,
         bool distribute_gain) const noexcept -> sos_array<Stages> {
    return step3a(z[z1], p1, z.swappop(z1), p, gain, distribute_gain);
  }

  constexpr auto
  step3a(cnum z1, cnum p1, carray<2 * Stages - 1> const& z,
         carray<2 * Stages - 1> const& p, value_type gain,
         bool distribute_gain) const noexcept -> sos_array<Stages> {
    return step4a(z1, p1, conjugate_index(z, z1), conjugate_index(p, p1), z, p,
                  gain, distribute_gain);
  }

  constexpr auto
  step4a(cnum z1, cnum p1, std::size_t z2, std::size_t p2,
         carray<2 * Stages - 1> const& z, carray<2 * Stages - 1> const& p,
         value_type gain, bool distribute_gain) const noexcept
      -> sos_array<Stages> {
    return finish(z1, p1, z[z2], p[p2], z.swappop(z2), p.swappop(p2), gain,
                  distribute_gain);
  }

  // We have two poles, either a conjugate pair or two real poles. We
  // need to find two zeros.
  constexpr auto
  step2b(cnum p1, std::size_t p2, carray<2 * Stages> const& z,
         carray<2 * Stages - 1> const& p, value_type gain,
         bool distribute_gain) const noexcept -> sos_array<Stages> {
    return step3b(p1, p[p2], z, p.swappop(p2), gain, distribute_gain);
  }

  constexpr auto
  step3b(cnum p1, cnum p2, carray<2 * Stages> const& z,
         carray<2 * Stages - 2> const& p, value_type gain,
         bool distribute_gain) const noexcept -> sos_array<Stages> {
    return step4b(p1, p2, nearest_index(z, p1), z, p, gain, distribute_gain);
  }

  constexpr auto
  step4b(cnum p1, cnum p2, std::size_t z1, carray<2 * Stages> const& z,
         carray<2 * Stages - 2> const& p, value_type gain,
         bool distribute_gain) const noexcept -> sos_array<Stages> {
    return step5b(p1, p2, z[z1], z.swappop(z1), p, gain, distribute_gain);
  }

  constexpr auto
  step5b(cnum p1, cnum p2, cnum z1, carray<2 * Stages - 1> const& z,
         carray<2 * Stages - 2> const& p, value_type gain,
         bool distribute_gain) const noexcept -> sos_array<Stages> {
    return step6b(p1, p2, z1,
                  z1.is_real() ? nearest_real_index(z, p1)
                               : conjugate_index(z, z1),
                  z, p, gain, distribute_gain);
  }

  constexpr auto
  step6b(cnum p1, cnum p2, cnum z1, std::size_t z2,
         carray<2 * Stages - 1> const& z, carray<2 * Stages - 2> const& p,
         value_type gain, bool distribute_gain) const noexcept
      -> sos_array<Stages> {
    return finish(z1, p1, z[z2], p2, z.swappop(z2), p, gain, distribute_gain);
  }

  constexpr auto
  finish(cnum z1, cnum p1, cnum z2, cnum p2, carray<2 * Stages - 2> const& z,
         carray<2 * Stages - 2> const& p, value_type gain,
         bool distribute_gain) const noexcept -> sos_array<Stages> {
    return zpk_to_sos<output_type, value_type, Stages - 1>{}(z, p, gain,
                                                             distribute_gain)
        .append(sos_array<1>{sos_section<output_type>{
            carray<2>{z1, z2}, carray<2>{p1, p2},
            Stages == 1 || distribute_gain ? gain : value_type{1}}});
  }

  template <std::size_t N>
  constexpr std::size_t next_pole_index(carray<N> const& p) const noexcept {
    return p.argmin(unit_circle_distance_less{});
  }

  template <std::size_t N>
  constexpr std::size_t
  next_real_pole_index(carray<N> const& p) const noexcept {
    return p.argmin(unit_circle_distance_real_less{});
  }

  template <std::size_t N>
  constexpr std::size_t
  nearest_index(carray<N> const& a,
                cmath::complex<value_type> const& z) const noexcept {
    return a.argmin(distance_less<value_type>{z});
  }

  template <std::size_t N>
  constexpr std::size_t
  nearest_real_index(carray<N> const& a,
                     cmath::complex<value_type> const& z) const noexcept {
    return a.argmin(distance_real_less<value_type>{z});
  }

  template <std::size_t N>
  constexpr std::size_t
  nearest_complex_index(carray<N> const& a,
                        cmath::complex<value_type> const& z) const noexcept {
    return a.argmin(distance_complex_less<value_type>{z});
  }

  template <std::size_t N>
  constexpr std::size_t
  conjugate_index(carray<N> const& a,
                  cmath::complex<value_type> const& z) const noexcept {
    return nearest_index(a, z.conj());
  }
};

template <typename FO, typename F>
class zpk_to_sos<FO, F, 0> {
 public:
  using value_type = F;
  using output_type = FO;
  using carray = cmath::vector<cmath::complex<value_type>, 0>;
  using sos_array = cmath::vector<sos_section<output_type>, 0>;

  constexpr auto operator()(carray const&, carray const&, value_type,
                            bool) const noexcept -> sos_array {
    return sos_array{};
  }
};

template <typename F, std::size_t N, std::size_t I = 0>
struct filter_chain {
  F operator()(cmath::vector<sos_section<F>, N> const& sos,
               std::array<sos_state<F>, N>& state, F x) const {
    return filter_chain<F, N, I + 1>{}(sos, state, sos[I].filter(state[I], x));
  }
};

template <typename F, std::size_t N>
struct filter_chain<F, N, N> {
  F operator()(cmath::vector<sos_section<F>, N> const&,
               std::array<sos_state<F>, N>&, F x) const {
    return x;
  }
};

template <typename F, std::size_t Order>
constexpr auto
butterworth_poles() noexcept -> cmath::vector<cmath::complex<F>, Order> {
  return -cmath::exp(detail::theta<F, Order>()());
}

} // namespace detail
} // namespace signal
} // namespace embedded
