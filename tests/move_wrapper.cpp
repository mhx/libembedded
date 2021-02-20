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

#include <array>

#include "embedded/function.h"
#include "embedded/move_wrapper.h"

#include <gtest/gtest.h>

using namespace embedded;

namespace {

struct move_only {
  int value;
  bool valid{true};

  move_only(int val) noexcept
      : value{val} {};

  move_only(move_only&& other) noexcept
      : value{std::move(other.value)} {
    other.valid = false;
  }
  move_only& operator=(move_only&& other) noexcept {
    value = std::move(other.value);
    valid = other.valid;
    other.valid = false;
    return *this;
  }

  move_only(move_only const&) = delete;
  move_only& operator=(move_only const&) = delete;

  explicit operator bool() const { return valid; }
};

int doit(function<int(int)> f) { return f(13); }

} // namespace

TEST(move_wrapper, basic) {
  move_wrapper<move_only> wrap(move_only(42));
  EXPECT_EQ(55, doit([=](int x) { return wrap->value + x; }));
}

TEST(move_wrapper, validity) {
  move_only probe(42);
  EXPECT_TRUE(probe);

  move_wrapper<move_only> wrap(std::move(probe));
  EXPECT_FALSE(probe);

  EXPECT_EQ(55, doit([=](int x) {
              EXPECT_TRUE(*wrap);
              return wrap->value + x;
            }));
}
