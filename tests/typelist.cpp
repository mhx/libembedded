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

#include <tuple>
#include <type_traits>

#include "embedded/typelist.h"

#include <gtest/gtest.h>

namespace {

struct A {};
struct B {};
struct C {};
struct D {};
struct V {
  virtual void foo() = 0;
};

using TL1 = embedded::typelist<>;
using TL2 = embedded::typelist<int>;
using TL3 = embedded::typelist<A, B>;
using TL4 = TL1::append<>;
using TL5 = TL2::append<float>;
using TL6 = TL3::append<C, D>;
using TL7 = TL6::append<V>;

using T1 = TL1::to<std::tuple>;
using T2 = TL2::to<std::tuple>;
using T3 = TL3::to<std::tuple>;
using T4 = TL4::to<std::tuple>;
using T5 = TL5::to<std::tuple>;
using T6 = TL6::to<std::tuple>;
using T7 = TL7::to<std::tuple>;

static_assert(0 == std::tuple_size<T1>::value, "");
static_assert(1 == std::tuple_size<T2>::value, "");
static_assert(2 == std::tuple_size<T3>::value, "");
static_assert(0 == std::tuple_size<T4>::value, "");
static_assert(2 == std::tuple_size<T5>::value, "");
static_assert(4 == std::tuple_size<T6>::value, "");
static_assert(5 == std::tuple_size<T7>::value, "");

static_assert(std::is_same<std::tuple_element<0, T2>::type, int>::value, "");
static_assert(std::is_same<std::tuple_element<0, T3>::type, A>::value, "");
static_assert(std::is_same<std::tuple_element<1, T3>::type, B>::value, "");
static_assert(std::is_same<std::tuple_element<0, T5>::type, int>::value, "");
static_assert(std::is_same<std::tuple_element<1, T5>::type, float>::value, "");
static_assert(std::is_same<std::tuple_element<0, T6>::type, A>::value, "");
static_assert(std::is_same<std::tuple_element<1, T6>::type, B>::value, "");
static_assert(std::is_same<std::tuple_element<2, T6>::type, C>::value, "");
static_assert(std::is_same<std::tuple_element<3, T6>::type, D>::value, "");
static_assert(std::is_same<std::tuple_element<4, T7>::type, V>::value, "");

} // namespace

TEST(typelist, basic) {}
