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

#ifdef __cpp_lib_is_invocable

template <class R, class Fn, class... Args>
using is_invocable_r = std::is_invocable_r<R, Fn, Args...>;

#else

namespace detail {

template <class R>
void accept(R);

template <class, class R, class Fn, class... Args>
struct is_invocable_helper : std::false_type {};

template <class Fn, class... Args>
struct is_invocable_helper<decltype(std::declval<Fn>()(std::declval<Args>()...),
                                    void()),
                           void, Fn, Args...> : std::true_type {};

template <class Fn, class... Args>
struct is_invocable_helper<decltype(std::declval<Fn>()(std::declval<Args>()...),
                                    void()),
                           const void, Fn, Args...> : std::true_type {};

template <class R, class Fn, class... Args>
struct is_invocable_helper<decltype(accept<R>(
                               std::declval<Fn>()(std::declval<Args>()...))),
                           R, Fn, Args...> : std::true_type {};

} // namespace detail

template <class R, class Fn, class... Args>
using is_invocable_r = detail::is_invocable_helper<void, R, Fn, Args...>;

#endif

} // namespace embedded
