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

#include "embedded/circular_buffer_adapter.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

namespace {

enum operation {
  CONSTRUCT,
  DESTRUCT,
  MOVE_CONSTRUCT,
  MOVE_ASSIGN,
  COPY_CONSTRUCT,
  COPY_ASSIGN
};

struct testdata {
  testdata(int x)
      : x_{x} {
#if LIBEMB_HAS_EXCEPTIONS
    if (x_ == 4711) {
      throw std::invalid_argument("test");
    }
#endif
    ops_.push_back(CONSTRUCT);
    if (!alive_.emplace(this).second) {
      std::terminate();
    }
  }
  ~testdata() {
    ops_.push_back(DESTRUCT);
    if (alive_.erase(this) != 1) {
      std::terminate();
    }
  }

  testdata(testdata&& other)
      : x_{other.x_} {
#if LIBEMB_HAS_EXCEPTIONS
    if (x_ == 4712) {
      throw std::invalid_argument("test");
    }
#endif
    ops_.push_back(MOVE_CONSTRUCT);
    if (!alive_.emplace(this).second) {
      std::terminate();
    }
  }

  testdata& operator=(testdata&& other) {
    ops_.push_back(MOVE_ASSIGN);
    if (&other != this) {
      x_ = other.x_;
    }
    return *this;
  }

  static void reset() {
    alive_.clear();
    ops_.clear();
  }

  static size_t alive() { return alive_.size(); }

  static bool expect_ops(std::initializer_list<operation> ops) {
    std::vector<operation> got;
    got.swap(ops_);
    return ops.size() == got.size() &&
           std::equal(ops.begin(), ops.end(), got.begin());
  }

  int x_{std::numeric_limits<int>::max()};
  static std::unordered_set<testdata*> alive_;
  static std::vector<operation> ops_;
};

std::vector<operation> testdata::ops_;
std::unordered_set<testdata*> testdata::alive_;

struct testdata_copyable {
  testdata_copyable(int x)
      : x_{x} {
#if LIBEMB_HAS_EXCEPTIONS
    if (x_ == 4711) {
      throw std::invalid_argument("test");
    }
#endif
    ops_.push_back(CONSTRUCT);
    if (!alive_.emplace(this).second) {
      std::terminate();
    }
  }
  ~testdata_copyable() {
    ops_.push_back(DESTRUCT);
    if (alive_.erase(this) != 1) {
      std::terminate();
    }
  }

  testdata_copyable(testdata_copyable&&) = delete;
  testdata_copyable& operator=(testdata_copyable&&) = delete;

  testdata_copyable(testdata_copyable const& other)
      : x_{other.x_} {
#if LIBEMB_HAS_EXCEPTIONS
    if (x_ == 4712) {
      throw std::invalid_argument("test");
    }
#endif
    ops_.push_back(COPY_CONSTRUCT);
    if (!alive_.emplace(this).second) {
      std::terminate();
    }
  }

  testdata_copyable& operator=(testdata_copyable const& other) {
    ops_.push_back(COPY_ASSIGN);
    if (&other != this) {
      x_ = other.x_;
    }
    return *this;
  }

  static void reset() {
    alive_.clear();
    ops_.clear();
  }

  static size_t alive() { return alive_.size(); }

  static bool expect_ops(std::initializer_list<operation> ops) {
    std::vector<operation> got;
    got.swap(ops_);
    return ops.size() == got.size() &&
           std::equal(ops.begin(), ops.end(), got.begin());
  }

  int x_;
  static std::unordered_set<testdata_copyable*> alive_;
  static std::vector<operation> ops_;
};

std::vector<operation> testdata_copyable::ops_;
std::unordered_set<testdata_copyable*> testdata_copyable::alive_;

} // namespace

TEST(circular_buffer_adapter, basic) {
  std::vector<uint8_t> raw;
  raw.resize(3);

  embedded::circular_buffer_adapter<uint8_t> cba(raw.data(), raw.size());

  EXPECT_EQ(3, cba.capacity());
  EXPECT_EQ(0, cba.size());
  EXPECT_EQ(3, cba.remaining());
  EXPECT_TRUE(cba.empty());

  cba.push_back(7);

  EXPECT_EQ(3, cba.capacity());
  EXPECT_EQ(1, cba.size());
  EXPECT_EQ(2, cba.remaining());
  EXPECT_EQ(7, cba.front());
  EXPECT_EQ(7, cba.back());

  cba.push_back(9);

  EXPECT_EQ(2, cba.size());
  EXPECT_EQ(7, cba.front());
  EXPECT_EQ(9, cba.back());

  cba.pop_front();

  EXPECT_EQ(1, cba.size());
  EXPECT_EQ(9, cba.front());
  EXPECT_EQ(9, cba.back());

  cba.pop_front();

  EXPECT_EQ(0, cba.size());

  std::fill_n(std::back_inserter(cba), cba.capacity(), 42);

  EXPECT_EQ(3, cba.size());
  EXPECT_EQ(0, cba.remaining());
  EXPECT_EQ(42, cba.front());
  EXPECT_EQ(42, cba.back());

  std::vector<uint8_t> ref{42, 42, 42};
  std::vector<uint8_t> tmp;
  for (auto v : cba) {
    tmp.push_back(v);
  }

  EXPECT_EQ(ref, tmp);

  std::reverse(ref.begin(), ref.end());
  std::copy(cba.rbegin(), cba.rend(), tmp.begin());

  EXPECT_EQ(ref, tmp);

  EXPECT_TRUE(cba.full());

  cba.clear();

  EXPECT_EQ(0, cba.size());
  EXPECT_TRUE(cba.empty());

  std::fill_n(std::back_inserter(cba), cba.capacity(), 0);
  std::iota(cba.begin(), cba.end(), 1);

  EXPECT_TRUE(cba.full());
  EXPECT_EQ(1, cba.front());
  EXPECT_EQ(3, cba.back());
}

TEST(circular_buffer_adapter, basic_const_on_mutable) {
  std::vector<uint8_t> raw{1, 2, 3};

  embedded::circular_buffer_adapter<uint8_t const> cba(raw.data(), raw.size(),
                                                       2, 2);

  EXPECT_EQ(3, cba.capacity());
  EXPECT_EQ(2, cba.size());
  EXPECT_EQ(3, cba.front());
  EXPECT_EQ(1, cba.back());

  EXPECT_EQ(2, cba.raw_index(cba.begin()));
  EXPECT_EQ(1, cba.raw_index(cba.end()));
  EXPECT_EQ(2, cba.end() - cba.begin());
  EXPECT_EQ(2, std::distance(cba.begin(), cba.end()));

  std::vector<uint8_t> ref{3, 1};
  std::vector<uint8_t> tmp;
  for (auto v : cba) {
    tmp.push_back(v);
  }

  EXPECT_EQ(ref, tmp);

  std::reverse(ref.begin(), ref.end());
  std::copy(cba.rbegin(), cba.rend(), tmp.begin());

  EXPECT_EQ(ref, tmp);
}

TEST(circular_buffer_adapter, basic_const_on_const) {
  std::vector<uint8_t> const raw{1, 2, 3};

  embedded::circular_buffer_adapter<uint8_t const> cba(raw.data(), raw.size(),
                                                       2, 2);

  EXPECT_EQ(3, cba.capacity());
  EXPECT_EQ(2, cba.size());
  EXPECT_EQ(3, cba.front());
  EXPECT_EQ(1, cba.back());

  EXPECT_EQ(2, cba.raw_index(cba.begin()));
  EXPECT_EQ(1, cba.raw_index(cba.end()));
  EXPECT_EQ(2, cba.end() - cba.begin());
  EXPECT_EQ(2, std::distance(cba.begin(), cba.end()));

  std::vector<uint8_t> ref{3, 1};
  std::vector<uint8_t> tmp;
  for (auto v : cba) {
    tmp.push_back(v);
  }

  EXPECT_EQ(ref, tmp);

  std::reverse(ref.begin(), ref.end());
  std::copy(cba.rbegin(), cba.rend(), tmp.begin());

  EXPECT_EQ(ref, tmp);
}

TEST(circular_buffer_adapter, type_checks) {
  using cba_t = embedded::circular_buffer_adapter<uint8_t>;
  static_assert(!std::is_copy_constructible<cba_t>::value, "copy-ctor");
  static_assert(!std::is_copy_assignable<cba_t>::value, "copy-assign");
  static_assert(std::is_move_constructible<cba_t>::value, "move-ctor");
  static_assert(std::is_move_assignable<cba_t>::value, "move-assign");
}

TEST(circular_buffer_adapter, create_destroy) {
  using cba_t = embedded::circular_buffer_adapter<testdata>;
  constexpr size_t num_items = 3;
  std::aligned_storage<num_items * sizeof(testdata), alignof(testdata)>::type
      buffer;

  testdata::reset();

  cba_t::size_type first;
  cba_t::size_type size;

  {
    cba_t cba(reinterpret_cast<testdata*>(&buffer), num_items);
    EXPECT_TRUE(testdata::expect_ops({}));

    cba.push_back(testdata(1));
    EXPECT_TRUE(testdata::expect_ops({CONSTRUCT, MOVE_CONSTRUCT, DESTRUCT}));

    cba.emplace_back(2);
    EXPECT_TRUE(testdata::expect_ops({CONSTRUCT}));

    cba.pop_back();
    EXPECT_TRUE(testdata::expect_ops({DESTRUCT}));

    cba.emplace_front(3);
    EXPECT_TRUE(testdata::expect_ops({CONSTRUCT}));

    cba.clear();
    EXPECT_TRUE(testdata::expect_ops({DESTRUCT, DESTRUCT}));

    EXPECT_EQ(0, testdata::alive());

    cba.emplace_back(1);
    cba.emplace_back(2);
    cba.emplace_back(3);
    cba.pop_front();
    cba.emplace_back(4);
    EXPECT_TRUE(testdata::expect_ops(
        {CONSTRUCT, CONSTRUCT, CONSTRUCT, DESTRUCT, CONSTRUCT}));

    first = cba.raw_index(cba.begin());
    size = cba.size();

    EXPECT_EQ(2, cba.front().x_);
    EXPECT_EQ(4, cba.back().x_);
  }

  EXPECT_TRUE(testdata::expect_ops({}));
  EXPECT_EQ(3, testdata::alive());
  EXPECT_EQ(1, first);
  EXPECT_EQ(3, size);

  {
    cba_t cba(reinterpret_cast<testdata*>(&buffer), num_items, first, size);
    EXPECT_TRUE(testdata::expect_ops({}));

    EXPECT_EQ(2, cba.front().x_);
    EXPECT_EQ(4, cba.back().x_);

    cba.pop_back();
    EXPECT_TRUE(testdata::expect_ops({DESTRUCT}));

    {
      auto td = std::move(cba[0]);
      EXPECT_TRUE(testdata::expect_ops({MOVE_CONSTRUCT}));
    }
    EXPECT_TRUE(testdata::expect_ops({DESTRUCT}));

    cba.clear();
    EXPECT_TRUE(testdata::expect_ops({DESTRUCT, DESTRUCT}));
  }
}

TEST(circular_buffer_adapter, raw_index) {
  std::vector<uint8_t> raw(3);
  embedded::circular_buffer_adapter<uint8_t> cba(raw.data(), raw.size());

  EXPECT_EQ(0, cba.raw_index(cba.begin()));
  EXPECT_EQ(0, cba.raw_index(cba.end()));

  cba.emplace_back(1);

  EXPECT_EQ(0, cba.raw_index(cba.begin()));
  EXPECT_EQ(1, cba.raw_index(cba.end()));

  cba.emplace_back(2);

  EXPECT_EQ(0, cba.raw_index(cba.begin()));
  EXPECT_EQ(2, cba.raw_index(cba.end()));

  cba.emplace_back(3);

  EXPECT_EQ(0, cba.raw_index(cba.begin()));
  EXPECT_EQ(0, cba.raw_index(cba.end()));
  EXPECT_EQ(1, cba.raw_index(cba.begin() + 1));
  EXPECT_EQ(2, cba.raw_index(cba.end() - 1));

  cba.pop_front();
  EXPECT_EQ(1, cba.raw_index(cba.begin()));
  EXPECT_EQ(0, cba.raw_index(cba.end()));

  cba.pop_front();
  EXPECT_EQ(2, cba.raw_index(cba.begin()));
  EXPECT_EQ(0, cba.raw_index(cba.end()));
  EXPECT_EQ(0, cba.raw_index(cba.begin() + 1));

  cba.emplace_back(4);
  EXPECT_EQ(2, cba.raw_index(cba.begin()));
  EXPECT_EQ(1, cba.raw_index(cba.end()));

  cba.emplace_back(5);
  EXPECT_EQ(2, cba.raw_index(cba.begin()));
  EXPECT_EQ(2, cba.raw_index(cba.end()));

  cba.pop_back();
  EXPECT_EQ(2, cba.raw_index(cba.begin()));
  EXPECT_EQ(1, cba.raw_index(cba.end()));

  cba.emplace_front(6);
  EXPECT_EQ(1, cba.raw_index(cba.begin()));
  EXPECT_EQ(1, cba.raw_index(cba.end()));
  EXPECT_EQ(0, cba.raw_index(cba.end() - 1));

  cba.pop_front();
  cba.pop_back();
  EXPECT_EQ(2, cba.raw_index(cba.begin()));
  EXPECT_EQ(0, cba.raw_index(cba.end()));

  cba.pop_back();
  EXPECT_EQ(2, cba.raw_index(cba.begin()));
  EXPECT_EQ(2, cba.raw_index(cba.end()));
  EXPECT_TRUE(cba.empty());
}

TEST(circular_buffer_adapter, mutable_iterator) {
  std::vector<uint8_t> raw(4);
  embedded::circular_buffer_adapter<uint8_t> cba(raw.data(), raw.size());

  EXPECT_TRUE(cba.begin() == cba.end());

  {
    auto it = cba.begin();

    static_assert(std::is_same<decltype(it[0]), uint8_t&>::value,
                  "subscript type");
    static_assert(std::is_same<decltype(*it), uint8_t&>::value, "deref type");
    static_assert(std::is_same<decltype(it.operator->()), uint8_t*>::value,
                  "pointer type");

    EXPECT_TRUE(it == cba.begin());
    EXPECT_FALSE(it != cba.begin());
    EXPECT_TRUE(it <= cba.begin());
    EXPECT_TRUE(it >= cba.begin());
    EXPECT_FALSE(it < cba.begin());
    EXPECT_FALSE(it > cba.begin());

    EXPECT_TRUE(it == cba.cbegin());
    EXPECT_FALSE(it != cba.cbegin());
    EXPECT_TRUE(it <= cba.cbegin());
    EXPECT_TRUE(it >= cba.cbegin());
    EXPECT_FALSE(it < cba.cbegin());
    EXPECT_FALSE(it > cba.cbegin());

    EXPECT_TRUE(it == cba.end());
  }

  {
    auto it = cba.end();

    static_assert(std::is_same<decltype(it[0]), uint8_t&>::value,
                  "subscript type");
    static_assert(std::is_same<decltype(*it), uint8_t&>::value, "deref type");
    static_assert(std::is_same<decltype(it.operator->()), uint8_t*>::value,
                  "pointer type");

    EXPECT_TRUE(it == cba.end());
    EXPECT_FALSE(it != cba.end());
    EXPECT_TRUE(it <= cba.end());
    EXPECT_TRUE(it >= cba.end());
    EXPECT_FALSE(it < cba.end());
    EXPECT_FALSE(it > cba.end());

    EXPECT_TRUE(it == cba.cend());
    EXPECT_FALSE(it != cba.cend());
    EXPECT_TRUE(it <= cba.cend());
    EXPECT_TRUE(it >= cba.cend());
    EXPECT_FALSE(it < cba.cend());
    EXPECT_FALSE(it > cba.cend());

    EXPECT_TRUE(it == cba.begin());
  }

  {
    auto it = cba.cbegin();

    static_assert(std::is_same<decltype(it[0]), uint8_t const&>::value,
                  "subscript type");
    static_assert(std::is_same<decltype(*it), uint8_t const&>::value,
                  "deref type");
    static_assert(
        std::is_same<decltype(it.operator->()), uint8_t const*>::value,
        "pointer type");

    EXPECT_TRUE(it == cba.begin());
    EXPECT_FALSE(it != cba.begin());
    EXPECT_TRUE(it <= cba.begin());
    EXPECT_TRUE(it >= cba.begin());
    EXPECT_FALSE(it < cba.begin());
    EXPECT_FALSE(it > cba.begin());

    EXPECT_TRUE(it == cba.cbegin());
    EXPECT_FALSE(it != cba.cbegin());
    EXPECT_TRUE(it <= cba.cbegin());
    EXPECT_TRUE(it >= cba.cbegin());
    EXPECT_FALSE(it < cba.cbegin());
    EXPECT_FALSE(it > cba.cbegin());

    EXPECT_TRUE(it == cba.end());
  }

  {
    auto it = cba.cend();

    static_assert(std::is_same<decltype(it[0]), uint8_t const&>::value,
                  "subscript type");
    static_assert(std::is_same<decltype(*it), uint8_t const&>::value,
                  "deref type");
    static_assert(
        std::is_same<decltype(it.operator->()), uint8_t const*>::value,
        "pointer type");

    EXPECT_TRUE(it == cba.end());
    EXPECT_FALSE(it != cba.end());
    EXPECT_TRUE(it <= cba.end());
    EXPECT_TRUE(it >= cba.end());
    EXPECT_FALSE(it < cba.end());
    EXPECT_FALSE(it > cba.end());

    EXPECT_TRUE(it == cba.cend());
    EXPECT_FALSE(it != cba.cend());
    EXPECT_TRUE(it <= cba.cend());
    EXPECT_TRUE(it >= cba.cend());
    EXPECT_FALSE(it < cba.cend());
    EXPECT_FALSE(it > cba.cend());

    EXPECT_TRUE(it == cba.begin());
  }

  cba.push_back(1);

  {
    auto it = cba.begin();

    EXPECT_TRUE(it == cba.begin());
    EXPECT_FALSE(it != cba.begin());
    EXPECT_TRUE(it <= cba.begin());
    EXPECT_TRUE(it >= cba.begin());
    EXPECT_FALSE(it < cba.begin());
    EXPECT_FALSE(it > cba.begin());

    EXPECT_TRUE(it == cba.cbegin());
    EXPECT_FALSE(it != cba.cbegin());
    EXPECT_TRUE(it <= cba.cbegin());
    EXPECT_TRUE(it >= cba.cbegin());
    EXPECT_FALSE(it < cba.cbegin());
    EXPECT_FALSE(it > cba.cbegin());

    EXPECT_FALSE(it == cba.end());
    EXPECT_TRUE(it != cba.end());
    EXPECT_TRUE(it < cba.end());
    EXPECT_TRUE(it <= cba.end());
    EXPECT_FALSE(it > cba.end());
    EXPECT_FALSE(it >= cba.end());
  }

  {
    auto it = cba.end();

    EXPECT_TRUE(it == cba.end());
    EXPECT_FALSE(it != cba.end());
    EXPECT_TRUE(it <= cba.end());
    EXPECT_TRUE(it >= cba.end());
    EXPECT_FALSE(it < cba.end());
    EXPECT_FALSE(it > cba.end());

    EXPECT_TRUE(it == cba.cend());
    EXPECT_FALSE(it != cba.cend());
    EXPECT_TRUE(it <= cba.cend());
    EXPECT_TRUE(it >= cba.cend());
    EXPECT_FALSE(it < cba.cend());
    EXPECT_FALSE(it > cba.cend());

    EXPECT_FALSE(it == cba.begin());
    EXPECT_TRUE(it != cba.begin());
    EXPECT_FALSE(it < cba.begin());
    EXPECT_FALSE(it <= cba.begin());
    EXPECT_TRUE(it > cba.begin());
    EXPECT_TRUE(it >= cba.begin());
  }

  {
    auto it = cba.begin();
    auto it2 = it++;

    EXPECT_TRUE(it2 == cba.begin());
    EXPECT_TRUE(it == cba.end());
  }

  {
    auto it = cba.begin();
    auto it2 = ++it;

    EXPECT_TRUE(it2 == cba.end());
    EXPECT_TRUE(it == it2);
  }

  {
    auto it = cba.end();
    auto it2 = it--;

    EXPECT_TRUE(it2 == cba.end());
    EXPECT_TRUE(it == cba.begin());
  }

  {
    auto it = cba.end();
    auto it2 = --it;

    EXPECT_TRUE(it2 == cba.begin());
    EXPECT_TRUE(it == it2);
  }

  cba.push_back(2);
  cba.push_back(3);
  EXPECT_FALSE(cba.full());

  cba.push_back(4);
  cba.pop_front();
  cba.push_back(5);
  EXPECT_TRUE(cba.full());

  {
    auto it = cba.begin();

    EXPECT_TRUE(it == cba.begin());
    EXPECT_FALSE(it != cba.begin());
    EXPECT_TRUE(it <= cba.begin());
    EXPECT_TRUE(it >= cba.begin());
    EXPECT_FALSE(it < cba.begin());
    EXPECT_FALSE(it > cba.begin());

    EXPECT_FALSE(it == cba.end());
    EXPECT_TRUE(it != cba.end());
    EXPECT_TRUE(it < cba.end());
    EXPECT_TRUE(it <= cba.end());
    EXPECT_FALSE(it > cba.end());
    EXPECT_FALSE(it >= cba.end());
  }

  {
    auto it = cba.end();

    EXPECT_TRUE(it == cba.end());
    EXPECT_FALSE(it != cba.end());
    EXPECT_TRUE(it <= cba.end());
    EXPECT_TRUE(it >= cba.end());
    EXPECT_FALSE(it < cba.end());
    EXPECT_FALSE(it > cba.end());

    EXPECT_FALSE(it == cba.begin());
    EXPECT_TRUE(it != cba.begin());
    EXPECT_FALSE(it < cba.begin());
    EXPECT_FALSE(it <= cba.begin());
    EXPECT_TRUE(it > cba.begin());
    EXPECT_TRUE(it >= cba.begin());
  }

  EXPECT_EQ(4, cba.end() - cba.begin());
  EXPECT_EQ(4, std::distance(cba.begin(), cba.end()));

  {
    auto it = std::find(cba.begin(), cba.end(), 5);

    EXPECT_EQ(3, it - cba.begin());
    EXPECT_EQ(5, *it);

    EXPECT_EQ(2, *(it - 3));
    it -= 2;

    EXPECT_EQ(3, *it);
    EXPECT_EQ(5, *(it + 2));
    EXPECT_EQ(5, *(2 + it));
    EXPECT_EQ(3, it[0]);
    EXPECT_EQ(4, it[1]);
    EXPECT_EQ(5, it[2]);

    it -= -2;
    EXPECT_EQ(3, it - cba.begin());
    EXPECT_EQ(5, *it);

    it += -2;
    EXPECT_EQ(1, it - cba.begin());
    EXPECT_EQ(3, *it);

    it += 1;
    EXPECT_EQ(2, it - cba.begin());
    EXPECT_EQ(4, *it);

    it += 2;
    EXPECT_EQ(4, it - cba.begin());
    EXPECT_TRUE(it == cba.end());

    it -= 1;
    EXPECT_EQ(3, it - cba.begin());
    EXPECT_EQ(5, *it);
  }
}

TEST(circular_buffer_adapter, const_iterator) {
  std::vector<uint8_t> raw{5, 2, 3, 4};
  embedded::circular_buffer_adapter<uint8_t const> cba(raw.data(), raw.size(),
                                                       1, 4);

  EXPECT_TRUE(cba.full());

  {
    auto it = cba.begin();

    static_assert(std::is_same<decltype(*it), uint8_t const&>::value,
                  "deref type");
    static_assert(
        std::is_same<decltype(it.operator->()), uint8_t const*>::value,
        "pointer type");

    EXPECT_TRUE(it == cba.begin());
    EXPECT_FALSE(it != cba.begin());
    EXPECT_TRUE(it <= cba.begin());
    EXPECT_TRUE(it >= cba.begin());
    EXPECT_FALSE(it < cba.begin());
    EXPECT_FALSE(it > cba.begin());

    EXPECT_TRUE(it == cba.cbegin());
    EXPECT_FALSE(it != cba.cbegin());
    EXPECT_TRUE(it <= cba.cbegin());
    EXPECT_TRUE(it >= cba.cbegin());
    EXPECT_FALSE(it < cba.cbegin());
    EXPECT_FALSE(it > cba.cbegin());

    EXPECT_FALSE(it == cba.end());
  }

  {
    auto it = cba.end();

    static_assert(std::is_same<decltype(*it), uint8_t const&>::value,
                  "deref type");
    static_assert(
        std::is_same<decltype(it.operator->()), uint8_t const*>::value,
        "pointer type");

    EXPECT_TRUE(it == cba.end());
    EXPECT_FALSE(it != cba.end());
    EXPECT_TRUE(it <= cba.end());
    EXPECT_TRUE(it >= cba.end());
    EXPECT_FALSE(it < cba.end());
    EXPECT_FALSE(it > cba.end());

    EXPECT_TRUE(it == cba.cend());
    EXPECT_FALSE(it != cba.cend());
    EXPECT_TRUE(it <= cba.cend());
    EXPECT_TRUE(it >= cba.cend());
    EXPECT_FALSE(it < cba.cend());
    EXPECT_FALSE(it > cba.cend());

    EXPECT_FALSE(it == cba.begin());
  }

  {
    auto it = cba.begin();

    EXPECT_TRUE(it == cba.begin());
    EXPECT_FALSE(it != cba.begin());
    EXPECT_TRUE(it <= cba.begin());
    EXPECT_TRUE(it >= cba.begin());
    EXPECT_FALSE(it < cba.begin());
    EXPECT_FALSE(it > cba.begin());

    EXPECT_FALSE(it == cba.end());
    EXPECT_TRUE(it != cba.end());
    EXPECT_TRUE(it < cba.end());
    EXPECT_TRUE(it <= cba.end());
    EXPECT_FALSE(it > cba.end());
    EXPECT_FALSE(it >= cba.end());
  }

  {
    auto it = cba.end();

    EXPECT_TRUE(it == cba.end());
    EXPECT_FALSE(it != cba.end());
    EXPECT_TRUE(it <= cba.end());
    EXPECT_TRUE(it >= cba.end());
    EXPECT_FALSE(it < cba.end());
    EXPECT_FALSE(it > cba.end());

    EXPECT_FALSE(it == cba.begin());
    EXPECT_TRUE(it != cba.begin());
    EXPECT_FALSE(it < cba.begin());
    EXPECT_FALSE(it <= cba.begin());
    EXPECT_TRUE(it > cba.begin());
    EXPECT_TRUE(it >= cba.begin());
  }

  EXPECT_EQ(4, cba.end() - cba.begin());
  EXPECT_EQ(4, std::distance(cba.begin(), cba.end()));

  {
    auto it = std::find(cba.begin(), cba.end(), 5);

    EXPECT_EQ(3, it - cba.begin());
    EXPECT_EQ(5, *it);

    EXPECT_EQ(2, *(it - 3));
    it -= 2;

    EXPECT_EQ(3, *it);
    EXPECT_EQ(5, *(it + 2));
    EXPECT_EQ(5, *(2 + it));
    EXPECT_EQ(3, it[0]);
    EXPECT_EQ(4, it[1]);
    EXPECT_EQ(5, it[2]);

    it -= -2;
    EXPECT_EQ(3, it - cba.begin());
    EXPECT_EQ(5, *it);

    it += -2;
    EXPECT_EQ(1, it - cba.begin());
    EXPECT_EQ(3, *it);

    it += 1;
    EXPECT_EQ(2, it - cba.begin());
    EXPECT_EQ(4, *it);

    it += 2;
    EXPECT_EQ(4, it - cba.begin());
    EXPECT_TRUE(it == cba.end());

    it -= 1;
    EXPECT_EQ(3, it - cba.begin());
    EXPECT_EQ(5, *it);
  }
}

template <typename T>
class copy_in_out_fixture : public ::testing::Test {};

using copy_in_out_types = ::testing::Types<uint8_t, int16_t, int32_t>;

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

TYPED_TEST_SUITE(copy_in_out_fixture, copy_in_out_types);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

TYPED_TEST(copy_in_out_fixture, copy_in_out) {
  std::vector<TypeParam> raw(10);
  embedded::circular_buffer_adapter<TypeParam> cba(raw.data(), raw.size());

  EXPECT_TRUE(cba.empty());

  std::vector<TypeParam> in;

  cba.copy_in_back(in.data(), in.size());

  EXPECT_TRUE(cba.empty());

  cba.copy_in_front(in.data(), in.size());

  EXPECT_TRUE(cba.empty());

  in.emplace_back(1);

  cba.copy_in_back(in.data(), in.size());

  //  1
  // --  --  --  --  --  --  --  --  --  --
  // b
  //     e

  EXPECT_FALSE(cba.empty());
  EXPECT_EQ(1, cba.size());

  in[0] = 2;

  cba.copy_in_front(in.data(), in.size());

  //  1                                   2
  // --  --  --  --  --  --  --  --  --  --
  //                                     b
  //     e

  EXPECT_EQ(2, cba.size());
  EXPECT_EQ(2, cba.front());
  EXPECT_EQ(1, cba.back());

  in.resize(5);
  std::iota(in.begin(), in.end(), 3);

  cba.copy_in_back(in.data(), in.size());

  //  1   3   4   5   6   7               2
  // --  --  --  --  --  --  --  --  --  --
  //                                     b
  //                         e

  EXPECT_EQ(7, cba.size());

  in.resize(3);
  std::iota(in.begin(), in.end(), 8);

  cba.copy_in_front(in.data(), in.size());

  //  1   3   4   5   6   7   8   9  10   2
  // --  --  --  --  --  --  --  --  --  --
  //                         b
  //                         e

  EXPECT_TRUE(cba.full());
  EXPECT_EQ(10, cba.size());

  std::vector<TypeParam> out;

  cba.copy_out_front(out.data(), out.size());

  EXPECT_TRUE(cba.full());

  cba.copy_out_back(out.data(), out.size());

  EXPECT_TRUE(cba.full());

  out.resize(1);

  cba.copy_out_front(out.data(), out.size());

  //  1   3   4   5   6   7       9  10   2
  // --  --  --  --  --  --  --  --  --  --
  //                             b
  //                         e

  EXPECT_FALSE(cba.full());
  EXPECT_EQ(9, cba.size());
  EXPECT_EQ(8, out[0]);

  cba.copy_out_back(out.data(), out.size());

  //  1   3   4   5   6           9  10   2
  // --  --  --  --  --  --  --  --  --  --
  //                             b
  //                     e

  EXPECT_EQ(8, cba.size());
  EXPECT_EQ(7, out[0]);

  out.resize(4);

  cba.copy_out_front(out.data(), out.size());

  //      3   4   5   6
  // --  --  --  --  --  --  --  --  --  --
  //     b
  //                     e

  EXPECT_EQ(4, cba.size());
  EXPECT_EQ(std::vector<TypeParam>({9, 10, 2, 1}), out);

  out.resize(3);

  cba.copy_out_back(out.data(), out.size());

  //      3
  // --  --  --  --  --  --  --  --  --  --
  //     b
  //         e

  EXPECT_EQ(1, cba.size());
  EXPECT_EQ(std::vector<TypeParam>({4, 5, 6}), out);

  in.resize(4);
  std::iota(in.begin(), in.end(), 11);

  cba.copy_in_front(in.data(), in.size());

  // 14   3                      11  12  13
  // --  --  --  --  --  --  --  --  --  --
  //                             b
  //         e

  EXPECT_EQ(5, cba.size());

  out.resize(5);

  cba.copy_out_back(out.data(), out.size());

  //
  // --  --  --  --  --  --  --  --  --  --
  //                             b
  //                             e

  EXPECT_TRUE(cba.empty());
  EXPECT_EQ(std::vector<TypeParam>({11, 12, 13, 14, 3}), out);

  std::iota(in.begin(), in.end(), 15);

  cba.copy_in_back(in.data(), in.size());

  // 18                          15  16  17
  // --  --  --  --  --  --  --  --  --  --
  //                             b
  //     e

  EXPECT_EQ(4, cba.size());

  out.resize(1);
  cba.copy_out_back(out.data(), out.size());

  //                             15  16  17
  // --  --  --  --  --  --  --  --  --  --
  //                             b
  // e

  EXPECT_EQ(3, cba.size());
  EXPECT_EQ(std::vector<TypeParam>({18}), out);

  std::iota(in.begin(), in.end(), 19);

  cba.copy_in_back(in.data(), in.size());

  // 19  20  21  22              15  16  17
  // --  --  --  --  --  --  --  --  --  --
  //                             b
  //                 e

  out.resize(3);
  cba.copy_out_front(out.data(), out.size());

  EXPECT_EQ(4, cba.size());
  EXPECT_EQ(std::vector<TypeParam>({15, 16, 17}), out);

  EXPECT_EQ(0, cba.raw_index(cba.begin()));
  EXPECT_EQ(4, cba.raw_index(cba.end()));
}

#if LIBEMB_HAS_EXCEPTIONS
TEST(circular_buffer_adapter, exceptions) {
  std::vector<uint8_t> raw(4);
  embedded::circular_buffer_adapter<uint8_t> cba(raw.data(), raw.size());

  EXPECT_THROW(cba.at(0), std::out_of_range);

  cba.push_back(42);
  EXPECT_NO_THROW(cba.at(0));
  EXPECT_THROW(cba.at(1), std::out_of_range);
}

TEST(circular_buffer_adapter, exception_guarantee) {
  constexpr size_t num_items = 4;
  std::aligned_storage<num_items * sizeof(testdata), alignof(testdata)>::type
      buffer;
  embedded::circular_buffer_adapter<testdata> cba(
      reinterpret_cast<testdata*>(&buffer), num_items);

  testdata::reset();

  EXPECT_TRUE(cba.empty());

  EXPECT_THROW(cba.emplace_back(4711), std::invalid_argument);
  EXPECT_TRUE(testdata::expect_ops({}));
  EXPECT_TRUE(cba.empty());

  EXPECT_NO_THROW(cba.emplace_back(42));
  EXPECT_FALSE(cba.empty());
  EXPECT_EQ(1, cba.size());
  EXPECT_TRUE(testdata::expect_ops({CONSTRUCT}));

  EXPECT_THROW(cba.emplace_front(4711), std::invalid_argument);
  EXPECT_TRUE(testdata::expect_ops({}));
  EXPECT_EQ(1, cba.size());

  EXPECT_NO_THROW(cba.emplace_front(43));
  EXPECT_FALSE(cba.empty());
  EXPECT_EQ(2, cba.size());
  EXPECT_TRUE(testdata::expect_ops({CONSTRUCT}));

  EXPECT_THROW(cba.push_front(testdata(4712)), std::invalid_argument);
  EXPECT_TRUE(testdata::expect_ops({CONSTRUCT, DESTRUCT}));
  EXPECT_EQ(2, cba.size());

  EXPECT_THROW(cba.push_back(testdata(4712)), std::invalid_argument);
  EXPECT_TRUE(testdata::expect_ops({CONSTRUCT, DESTRUCT}));
  EXPECT_EQ(2, cba.size());

  cba.clear();
  EXPECT_EQ(0, testdata::alive());
}

TEST(circular_buffer_adapter, exception_guarantee_copyable) {
  constexpr size_t num_items = 4;
  std::aligned_storage<num_items * sizeof(testdata_copyable),
                       alignof(testdata_copyable)>::type buffer;
  embedded::circular_buffer_adapter<testdata_copyable> cba(
      reinterpret_cast<testdata_copyable*>(&buffer), num_items);

  testdata_copyable::reset();

  EXPECT_TRUE(cba.empty());

  EXPECT_THROW(cba.emplace_back(4711), std::invalid_argument);
  EXPECT_TRUE(testdata_copyable::expect_ops({}));
  EXPECT_TRUE(cba.empty());

  EXPECT_NO_THROW(cba.emplace_back(42));
  EXPECT_FALSE(cba.empty());
  EXPECT_EQ(1, cba.size());
  EXPECT_TRUE(testdata_copyable::expect_ops({CONSTRUCT}));

  EXPECT_THROW(cba.emplace_front(4711), std::invalid_argument);
  EXPECT_TRUE(testdata_copyable::expect_ops({}));
  EXPECT_EQ(1, cba.size());

  EXPECT_NO_THROW(cba.emplace_front(43));
  EXPECT_FALSE(cba.empty());
  EXPECT_EQ(2, cba.size());
  EXPECT_TRUE(testdata_copyable::expect_ops({CONSTRUCT}));

  {
    testdata_copyable orig(4712);
    EXPECT_TRUE(testdata_copyable::expect_ops({CONSTRUCT}));

    EXPECT_THROW(cba.push_front(orig), std::invalid_argument);
    EXPECT_EQ(2, cba.size());

    EXPECT_THROW(cba.push_back(orig), std::invalid_argument);
    EXPECT_EQ(2, cba.size());
  }
  EXPECT_TRUE(testdata_copyable::expect_ops({DESTRUCT}));

  cba.clear();
  EXPECT_EQ(0, testdata_copyable::alive());
}
#endif
