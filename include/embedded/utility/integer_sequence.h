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

namespace embedded {

template <typename T, T... Ints>
struct integer_sequence {
  using value_type = T;
  static constexpr std::size_t size() { return sizeof...(Ints); }
};

template <std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

template <typename T, std::size_t N, T... Is>
struct make_integer_sequence : make_integer_sequence<T, N - 1, N - 1, Is...> {};

template <typename T, T... Is>
struct make_integer_sequence<T, 0, Is...> : integer_sequence<T, Is...> {};

template <typename T, T B, T E, T... Is>
struct make_integer_range : make_integer_range<T, B, E - 1, E - 1, Is...> {};

template <typename T, T B, T... Is>
struct make_integer_range<T, B, B, Is...> : integer_sequence<T, Is...> {};

template <std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template <std::size_t B, std::size_t E>
using make_index_range = make_integer_range<std::size_t, B, E>;

template <typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

} // namespace embedded
