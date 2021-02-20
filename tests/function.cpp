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
#include "embedded/variant.h"

#if LIBEMB_HAS_EXCEPTIONS
#include <stdexcept>
#endif

#include <gtest/gtest.h>

using namespace embedded;

namespace {

struct tracer {
  static size_t ctor;
  static size_t dtor;
  static size_t move_ctor;
  static size_t move_assign;

  static void reset() {
    ctor = 0;
    dtor = 0;
    move_ctor = 0;
    move_assign = 0;
  }

  int value;

  tracer(int val) noexcept
      : value(val) {
    ctor++;
  };
  ~tracer() { dtor++; };

  tracer(tracer const&) = delete;
  tracer& operator=(tracer const&) = delete;

  tracer(tracer&& other) noexcept
      : value(std::move(other.value)) {
    move_ctor++;
  }
  tracer& operator=(tracer&& other) noexcept {
    value = std::move(other.value);
    move_ctor++;
    return *this;
  }

  int operator()(int x) const { return x + value; }
};

size_t tracer::ctor;
size_t tracer::dtor;
size_t tracer::move_ctor;
size_t tracer::move_assign;

template <class T>
struct store {
  T value{};

  T const& operator()() const { return value; }
  void operator()(T const& val) { value = val; }
};

struct test {
  int x;

  int y(int z) { return x + z; }
};

int doit(function<int(int)> f) { return f(42); }

int doit2(function<int(test, int)> f) {
  test t;
  t.x = 13;
  return f(t, 42);
}

} // namespace

TEST(function, basic) {
  uintptr_t a = 1, b = 2, c = 3;
  EXPECT_EQ(6 * 42, doit([=](int x) { return a * b * c * x; }));
  EXPECT_EQ(55, doit2(&test::y));
}

TEST(function, layout) {
  EXPECT_EQ(4 * sizeof(void*), sizeof(function<int(int)>));
  EXPECT_LE(alignof(int), alignof(function<int(int)>));
}

TEST(function, const_fun) {
  store<int> s;
  s(5);

  function<int() const> getter = std::ref(s);
  function<void(int)> setter = std::ref(s);

  EXPECT_EQ(5, getter());
  setter(13);
  EXPECT_EQ(13, getter());
  setter(31);
  EXPECT_EQ(31, getter());
}

TEST(function, mutable_fun) {
  int num = 42;
  function<int()> counter{[num]() mutable { return num++; }};
  EXPECT_EQ(42, counter());
  EXPECT_EQ(43, counter());

  EXPECT_FALSE(function<int(void)>());

  EXPECT_TRUE(counter);
  counter = nullptr;
  EXPECT_FALSE(counter);
}

TEST(function, moveonly) {
  tracer::reset();
  tracer probe(5);

  EXPECT_EQ(1, tracer::ctor);
  EXPECT_EQ(0, tracer::move_ctor);
  EXPECT_EQ(0, tracer::move_assign);
  EXPECT_EQ(0, tracer::dtor);

  EXPECT_EQ(47, doit(std::move(probe)));

  EXPECT_EQ(1, tracer::ctor);
  EXPECT_EQ(1, tracer::move_ctor);
  EXPECT_EQ(0, tracer::move_assign);
  EXPECT_EQ(1, tracer::dtor);
}

TEST(function, variant) {
  using call1 = function<int(int)>;
  using call2 = function<int(int, int)>;
  variant<call1, call2> var;

  EXPECT_EQ(0, var.index());
  EXPECT_FALSE(get<call1>(var));
#if LIBEMB_HAS_EXCEPTIONS
  EXPECT_THROW(get<call2>(var), std::exception);
#else
  EXPECT_DEATH(get<call2>(var), "terminate");
#endif

  var = [](int x) { return 13 + x; };

  EXPECT_EQ(0, var.index());
  EXPECT_TRUE(get<call1>(var));

  struct visitor {
    int arg;
    int res{0};
    explicit visitor(int arg)
        : arg(arg) {}
    void operator()(call1 c) { res = c(arg); }
    void operator()(call2&& c) { res = c(arg, 42); }
  };

  visitor v(5);

  visit(v, std::move(var));

  EXPECT_EQ(18, v.res);
  EXPECT_FALSE(get<call1>(var));

  var = [](int x, int y) { return 2 + x + y; };

  EXPECT_EQ(1, var.index());
  EXPECT_TRUE(get<call2>(var));

  visit(v, std::move(var));

  EXPECT_EQ(49, v.res);
  EXPECT_TRUE(get<call2>(var));
}

TEST(function, exception) {
  function<void()> fun1{[] {}};
#if LIBEMB_HAS_EXCEPTIONS
  function<void()> fun2{[] { throw std::runtime_error("boo"); }};
#endif

  fun1();

#if LIBEMB_HAS_EXCEPTIONS
  EXPECT_THROW(fun2(), std::runtime_error);
#endif

  fun1 = nullptr;

#if LIBEMB_HAS_EXCEPTIONS
  EXPECT_THROW(fun1(), std::bad_function_call);
#else
  EXPECT_DEATH(fun1(), "terminate");
#endif

#if LIBEMB_HAS_EXCEPTIONS
  auto dummy = std::move(fun2);
  EXPECT_THROW(fun2(), std::bad_function_call);
  EXPECT_THROW(dummy(), std::runtime_error);
#else
  fun1 = [] {};
  fun1();
  auto dummy = std::move(fun1);
  EXPECT_DEATH(fun1(), "terminate");
  dummy();
#endif
}
