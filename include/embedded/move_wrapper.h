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
#include <utility>

namespace embedded {

template <typename T>
class move_wrapper {
 public:
  explicit move_wrapper(T&& t) noexcept(std::is_nothrow_constructible<T>::value)
      : val_{std::forward<T>(t)} {}

  move_wrapper(move_wrapper const& other) noexcept(
      std::is_nothrow_move_constructible<T>::value)
      : val_{std::move(const_cast<move_wrapper&>(other).val_)} {}
  move_wrapper(move_wrapper&& other) noexcept(
      std::is_nothrow_move_constructible<T>::value)
      : val_{std::move(other.val_)} {}

  T const& operator*() const { return val_; }
  T& operator*() { return val_; }

  T const* operator->() const { return &val_; }
  T* operator->() { return &val_; }

 private:
  T val_;
};

} // namespace embedded
