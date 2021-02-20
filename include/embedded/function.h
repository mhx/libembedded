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

#include <functional>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "config.h"
#include "type_traits.h"

namespace embedded {

template <typename Signature, size_t Capacity = 3 * sizeof(void*),
          size_t Alignment = alignof(void*)>
class function;

namespace detail {

template <typename T>
struct wrap {
  using type = T;
};

enum class oper { move, destroy };

template <typename Signature>
class vtbl;

template <typename Return, typename... Args>
class vtbl<Return(Args...)> {
 public:
  using storage_t = void*;

  constexpr vtbl() noexcept {}

  template <typename T>
  explicit constexpr vtbl(wrap<T>) noexcept
      : call{&vt_call<T>}
      , proc{&vt_proc<T>} {}

 private:
  static Return empty_call(storage_t, Args&&...) {
#if LIBEMB_HAS_EXCEPTIONS
    throw std::bad_function_call();
#else
    std::terminate();
#endif
  }

  static void empty_proc(oper, storage_t, storage_t) noexcept {}

  template <typename T>
  static Return vt_call(storage_t s, Args&&... args) {
    return (*static_cast<T*>(s))(static_cast<Args&&>(args)...);
  }

  template <typename T>
  static void vt_proc(oper op, storage_t dst, storage_t src) noexcept {
    switch (op) {
    case oper::move:
      ::new (dst) T{std::move(*static_cast<T*>(src))};
      // fallthrough
    case oper::destroy:
      static_cast<T*>(dst)->~T();
      break;
    }
  }

 public:
  decltype(&empty_call) const call{&empty_call};
  decltype(&empty_proc) const proc{&empty_proc};
};

template <typename Signature>
struct empty_vtbl {
  static vtbl<Signature> constexpr value{};
};

template <typename Signature>
constexpr vtbl<Signature> empty_vtbl<Signature>::value;

template <typename Signature, typename T>
struct typed_vtbl {
  static vtbl<Signature> constexpr value{wrap<T>{}};
};

template <typename Signature, typename T>
constexpr vtbl<Signature> typed_vtbl<Signature, T>::value;

template <typename>
struct is_function : std::false_type {};
template <typename S, size_t C, size_t A>
struct is_function<function<S, C, A>> : std::true_type {};

template <typename Signature>
struct function_traits;

template <typename Return, typename... Args>
struct function_traits<Return(Args...)> {
  template <typename T>
  using is_invocable = is_invocable_r<Return, T&, Args...>;
  using signature = Return(Args...);

  Return operator()(Args... args) {
    auto fn = static_cast<function<Return(Args...)>*>(this);
    return fn->vtbl_->call(std::addressof(fn->storage_),
                           std::forward<Args>(args)...);
  }
};

template <typename Return, typename... Args>
struct function_traits<Return(Args...) const> {
  template <typename T>
  using is_invocable = is_invocable_r<Return, const T&, Args...>;
  using signature = Return(Args...);

  Return operator()(Args... args) const {
    auto fn = static_cast<function<Return(Args...) const> const*>(this);
    return fn->vtbl_->call(std::addressof(fn->storage_),
                           std::forward<Args>(args)...);
  }
};

} // namespace detail

/**
 * A const-correct, move-only function wrapper with in-place storage
 *
 * This function wrapper is largely inspired by `folly::Function` [1],
 * which is another const-correct, move-only function wrapper. I had a
 * few discussions about folly::Function with the author during its
 * inception and have used it extensively in the past. However, it's
 * unsuitable as-is for embedded systems without dynamic memory
 * allocation, and it requires at least C++14 to build.
 *
 * The code also incorporates ideas from SG14's `inplace_function` [2],
 * which is a proposed function wrapper with only in-place storage.
 *
 * The implementation only requires C++11 to build. Also, by default,
 * objects are a lot smaller than `folly::Function` objects, and they
 * only use a single pointer internally instead of two, at the cost of
 * another level of indirection.
 *
 * [1] https://github.com/facebook/folly/blob/master/folly/docs/Function.md
 * [2]
 * https://github.com/WG21-SG14/SG14/blob/master/Docs/Proposals/NonAllocatingStandardFunction.pdf
 */
template <typename Signature, size_t Capacity, size_t Alignment>
class function final : private detail::function_traits<Signature> {
 public:
  using traits = detail::function_traits<Signature>;
  using signature = typename traits::signature;
  using vtbl_t = detail::vtbl<signature>;
  using empty_vtbl = detail::empty_vtbl<signature>;
  template <typename T>
  using typed_vtbl = detail::typed_vtbl<signature, T>;

  friend traits;

  function() noexcept
      : vtbl_{&empty_vtbl::value} {}

  function(std::nullptr_t) noexcept
      : function() {}

  function(function const&) = delete;
  function& operator=(function const&) = delete;

  ~function() {
    vtbl_->proc(detail::oper::destroy, std::addressof(storage_), nullptr);
  }

  template <typename T, typename C = typename std::decay<T>::type,
            typename = typename std::enable_if<
                !detail::is_function<C>::value &&
                traits::template is_invocable<C>::value>::type>
  function(T&& fun) noexcept
      : vtbl_{&typed_vtbl<C>::value} {
    static_assert(
        std::is_nothrow_move_constructible<C>::value,
        "function<> can only be used with nothrow move-constructible types");
    static_assert(sizeof(C) <= Capacity,
                  "function<> storage too small for this type");
    static_assert(Alignment % alignof(C) == 0,
                  "function<> alignment too small for this type");

    ::new (std::addressof(storage_)) C{std::forward<T>(fun)};
  }

  function(function&& other) noexcept
      : vtbl_(other.vtbl_) {
    other.vtbl_ = &empty_vtbl::value;
    vtbl_->proc(detail::oper::move, std::addressof(storage_),
                std::addressof(other.storage_));
  }

  function& operator=(function&& other) noexcept {
    this->~function();
    ::new (this) function(std::move(other));
    return *this;
  }

  function& operator=(std::nullptr_t) noexcept {
    *this = function();
    return *this;
  }

  template <
      typename M, typename C,
      typename = decltype(function(std::mem_fn(static_cast<M C::*>(nullptr))))>
  function(M C::*pm) noexcept
      : function() {
    if (pm) {
      *this = std::mem_fn(pm);
    }
  }

  template <typename M, typename C>
  auto operator=(M C::*pm) noexcept -> decltype(operator=(std::mem_fn(pm))) {
    if (pm) {
      *this = std::mem_fn(pm);
    } else {
      *this = function();
    }
    return *this;
  }

  explicit operator bool() const noexcept {
    return vtbl_ != &empty_vtbl::value;
  }

  using traits::operator();

 private:
  vtbl_t const* vtbl_;
  alignas(Alignment) char mutable storage_[Capacity];
};

} // namespace embedded
