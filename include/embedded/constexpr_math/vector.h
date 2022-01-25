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
#include <type_traits>

#include "../type_traits/conjunction.h"
#include "../utility/integer_sequence.h"

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_suppress=Pe540
#endif
// clang-format on

namespace embedded {
namespace cmath {

template <typename T, std::size_t Size>
class vector;

namespace detail {

template <typename F>
class multiply {
 public:
  constexpr multiply(F x)
      : x_{x} {}

  template <typename T>
  constexpr auto operator()(T const& v) const noexcept -> T {
    return x_ * v;
  }

 private:
  F x_;
};

template <typename F>
class add {
 public:
  constexpr add(F x)
      : x_{x} {}

  template <typename T>
  constexpr auto operator()(T const& v) const noexcept -> T {
    return x_ + v;
  }

 private:
  F x_;
};

class negate {
 public:
  template <typename T>
  constexpr auto operator()(T const& v) const noexcept -> T {
    return -v;
  }
};

template <typename F>
class divide {
 public:
  constexpr divide(F x)
      : x_{x} {}

  template <typename T>
  constexpr auto operator()(T const& v) const noexcept -> T {
    return x_ / v;
  }

 private:
  F x_;
};

template <typename F>
class divide_by {
 public:
  constexpr divide_by(F x)
      : x_{x} {}

  template <typename T>
  constexpr auto operator()(T const& v) const noexcept -> T {
    return v / x_;
  }

 private:
  F x_;
};

class exp_functor {
 public:
  template <typename T>
  constexpr auto operator()(T const& v) const noexcept -> T {
    return exp(v);
  }
};

class prod_reducer {
 public:
  template <typename F>
  constexpr auto operator()(F const& a, F const& v) const noexcept -> F {
    return a * v;
  }
};

template <typename T, std::size_t Size, std::size_t I = 0>
struct compare {
  constexpr int operator()(T const& a, T const& b) const noexcept {
    return a[I] < b[I] ? -1 : a[I] > b[I] ? 1 : compare<T, Size, I + 1>()(a, b);
  }
};

template <typename T, std::size_t Size>
struct compare<T, Size, Size> {
  constexpr int operator()(T const&, T const&) const noexcept { return 0; }
};

template <typename T, std::size_t Size, typename Pred, std::size_t I = 0>
struct argmin {
  constexpr std::size_t operator()(T const& a, Pred const& pred,
                                   std::size_t min_index = 0) const noexcept {
    return argmin<T, Size, Pred, I + 1>()(
        a, pred, pred(a[I], a[min_index]) ? I : min_index);
  }
};

template <typename T, std::size_t Size, typename Pred>
struct argmin<T, Size, Pred, Size> {
  constexpr std::size_t
  operator()(T const&, Pred const&, std::size_t min_index = 0) const noexcept {
    return min_index;
  }
};

template <typename T, std::size_t Size, typename Pred, std::size_t I = 0>
struct count {
  constexpr std::size_t
  operator()(T const& a, Pred const& pred) const noexcept {
    return pred(a[I]) + count<T, Size, Pred, I + 1>()(a, pred);
  }
};

template <typename T, std::size_t Size, typename Pred>
struct count<T, Size, Pred, Size> {
  constexpr std::size_t operator()(T const&, Pred const&) const noexcept {
    return 0;
  }
};

template <typename T, std::size_t Size, typename Pred, std::size_t I = 0>
struct sort {
  constexpr auto
  operator()(vector<T, Size - I> const& a, Pred const& pred) const noexcept
      -> vector<T, Size - I> {
    return sort_min(a, pred, a.argmin(pred));
  }

 private:
  constexpr auto
  sort_min(vector<T, Size - I> const& a, Pred const& pred,
           std::size_t min) const noexcept -> vector<T, Size - I> {
    return vector<T, 1>{a[min]}.append(
        sort<T, Size, Pred, I + 1>{}(a.swappop(min), pred));
  }
};

template <typename T, std::size_t Size, typename Pred>
struct sort<T, Size, Pred, Size> {
  constexpr auto operator()(vector<T, 0> const& a, Pred const&) const noexcept
      -> vector<T, 0> {
    return a;
  }
};

template <typename T>
class constant {
 public:
  constexpr constant(T c = T{})
      : c_{c} {}

  constexpr auto operator()(int) const noexcept -> T { return c_; }

 private:
  T c_;
};

} // namespace detail

template <typename T, std::size_t Size>
struct vector_traits {
  using underlying_type = T[Size];

  static constexpr T const*
  pointer(underlying_type const& x, std::size_t i) noexcept {
    return &x[i];
  }
};

template <typename T>
struct vector_traits<T, 0> {
  struct underlying_type {};

  static constexpr T const*
  pointer(underlying_type const&, std::size_t) noexcept {
    return nullptr;
  }
};

template <typename T, std::size_t Size>
class vector {
 public:
  using value_type = T;
  using size_type = std::size_t;
  using const_reference = T const&;
  using traits_type = vector_traits<T, Size>;

  template <typename T2, size_t S2>
  friend class vector;

  template <typename... Args,
            typename = typename std::enable_if<conjunction<
                std::is_convertible<Args, value_type>...>::value>::type>
  constexpr vector(Args&&... args)
      : items_{args...} {}

  template <typename T2, typename = typename std::enable_if<
                             std::is_convertible<T2, value_type>::value>::type>
  constexpr vector(vector<T2, Size> const& other)
      : vector{other, make_index_sequence<Size>{}} {}

  template <typename Fn>
  static constexpr auto create(Fn const& fn) noexcept -> vector {
    return create_impl(fn, make_index_sequence<Size>{});
  }

  static constexpr auto full(T const& value) noexcept -> vector {
    return create(detail::constant<T>{value});
  }

  static constexpr auto ones() noexcept -> vector { return full(T{1}); }

  static constexpr auto zeros() noexcept -> vector { return full(T{0}); }

  constexpr const_reference operator[](size_type i) const noexcept {
    return *traits_type::pointer(items_, i);
  }

  static constexpr size_type size() noexcept { return Size; }

  template <typename T2 = value_type, typename Fn>
  constexpr auto transform(Fn const& fn) const noexcept -> vector<T2, Size> {
    return transform_impl<T2>(fn, make_index_sequence<Size>{});
  }

  template <typename Fn>
  constexpr auto
  reduce(Fn const& fn, value_type initial = value_type{1}) const noexcept
      -> value_type {
    return Size > 0 ? reduce_impl(initial, fn, Size - 1) : initial;
  }

  template <std::size_t S2>
  constexpr auto append(vector<value_type, S2> const& other) const noexcept
      -> vector<value_type, Size + S2> {
    return append_impl(other, make_index_sequence<Size>{},
                       make_index_sequence<S2>{});
  }

  template <std::size_t Pos, std::size_t Count>
  constexpr auto subvector() const noexcept -> vector<value_type, Count> {
    static_assert(Pos + Count <= Size, "invalid subvector range");
    return subvector_impl<Count>(make_index_range<Pos, Pos + Count>{});
  }

  constexpr bool operator==(vector const& other) const noexcept {
    return detail::compare<vector, Size>()(*this, other) == 0;
  }

  template <typename Pred>
  constexpr std::size_t argmin(Pred const& pred) const noexcept {
    return detail::argmin<vector, Size, Pred>()(*this, pred);
  }

  template <typename Pred>
  constexpr std::size_t count(Pred const& pred) const noexcept {
    return detail::count<vector, Size, Pred>()(*this, pred);
  }

  template <std::size_t Pos, std::size_t Count = 1>
  constexpr auto erase() const noexcept -> vector<value_type, Size - Count> {
    return subvector<0, Pos>().append(
        subvector<Pos + Count, Size - (Pos + Count)>());
  }

  constexpr auto swap(std::size_t a, std::size_t b) const noexcept -> vector {
    return swap_impl(a, b, make_index_sequence<Size>{});
  }

  constexpr auto
  swappop(std::size_t i) const noexcept -> vector<value_type, Size - 1> {
    return swap(i, 0).template erase<0>();
  }

  template <typename Pred>
  constexpr auto sort(Pred const& pred) const noexcept -> vector {
    return detail::sort<value_type, Size, Pred>{}(*this, pred);
  }

 private:
  template <typename T2, std::size_t... Ints,
            typename = typename std::enable_if<
                std::is_convertible<T2, value_type>::value>::type>
  constexpr vector(vector<T2, Size> const& other,
                   index_sequence<Ints...>) noexcept
      : items_{static_cast<value_type>(other[Ints])...} {}

  constexpr std::size_t
  swap_idx(std::size_t a, std::size_t b, std::size_t i) const noexcept {
    return i == a ? b : i == b ? a : i;
  }

  template <std::size_t... Ints>
  constexpr auto swap_impl(std::size_t a, std::size_t b,
                           index_sequence<Ints...>) const noexcept -> vector {
    return vector{(*this)[swap_idx(a, b, Ints)]...};
  }

  template <std::size_t Count, std::size_t... Ints>
  constexpr auto subvector_impl(index_sequence<Ints...>) const noexcept
      -> vector<value_type, Count> {
    return vector<value_type, Count>{(*this)[Ints]...};
  }

  template <typename Fn, std::size_t... Ints>
  static constexpr auto
  create_impl(Fn const& fn, index_sequence<Ints...>) noexcept -> vector {
    return vector{fn(Ints)...};
  }

  template <std::size_t S2, std::size_t... Ints1, std::size_t... Ints2>
  constexpr auto
  append_impl(vector<value_type, S2> const& other, index_sequence<Ints1...>,
              index_sequence<Ints2...>) const noexcept
      -> vector<value_type, Size + S2> {
    return vector<value_type, Size + S2>{(*this)[Ints1]..., other[Ints2]...};
  }

  template <typename Fn>
  constexpr auto
  reduce_impl(value_type value, Fn const& fn, std::size_t i) const noexcept
      -> value_type {
    return i > 0 ? reduce_impl(fn(value, (*this)[i]), fn, i - 1)
                 : fn(value, (*this)[i]);
  }

  template <typename T2, typename Fn, std::size_t... Ints>
  constexpr auto
  transform_impl(Fn const& fn, index_sequence<Ints...>) const noexcept
      -> vector<T2, Size> {
    return vector<T2, Size>{fn((*this)[Ints])...};
  }

  typename traits_type::underlying_type items_;
};

template <typename T, typename Fn, std::size_t... Ints>
constexpr auto make_vector(Fn const& fn, index_sequence<Ints...>) noexcept
    -> vector<T, sizeof...(Ints)> {
  return vector<T, sizeof...(Ints)>{fn(Ints)...};
}

template <typename T, std::size_t Size>
constexpr auto operator-(vector<T, Size> const& a) noexcept -> vector<T, Size> {
  return a.transform(detail::negate());
}

template <typename T1, typename T2, std::size_t Size>
constexpr auto
operator*(T1 const& x, vector<T2, Size> const& a) noexcept -> vector<T2, Size> {
  return a.transform(detail::multiply<T1>(x));
}

template <typename T1, typename T2, std::size_t Size>
constexpr auto
operator+(T1 const& x, vector<T2, Size> const& a) noexcept -> vector<T2, Size> {
  return a.transform(detail::add<T1>(x));
}

template <typename T1, typename T2, std::size_t Size>
constexpr auto
operator/(T1 const& x, vector<T2, Size> const& a) noexcept -> vector<T2, Size> {
  return a.transform(detail::divide<T1>(x));
}

template <typename T1, typename T2, std::size_t Size>
constexpr auto
operator/(vector<T1, Size> const& a, T2 const& x) noexcept -> vector<T1, Size> {
  return a.transform(detail::divide_by<T2>(x));
}

template <typename T, std::size_t Size>
constexpr auto exp(vector<T, Size> const& a) noexcept -> vector<T, Size> {
  return a.transform(detail::exp_functor());
}

template <typename F, std::size_t Size>
constexpr auto prod(cmath::vector<F, Size> const& a) noexcept -> F {
  return a.reduce(detail::prod_reducer());
}

} // namespace cmath
} // namespace embedded

// clang-format off
#ifdef __IAR_SYSTEMS_ICC__
#pragma diag_default=Pe540
#endif
// clang-format on
