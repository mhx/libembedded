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

#include "embedded/varint.h"

#include <numeric>
#include <vector>

#include <gtest/gtest.h>

TEST(varint, basic_unsigned) {
  using namespace embedded;

  EXPECT_EQ(1, varint::size(0U));
  EXPECT_EQ(1, varint::size(1U));
  EXPECT_EQ(1, varint::size(127U));
  EXPECT_EQ(2, varint::size(128U));
  EXPECT_EQ(2, varint::size(16383U));
  EXPECT_EQ(3, varint::size(16384U));
  EXPECT_EQ((8 * sizeof(unsigned) + 6) / 7,
            varint::size(std::numeric_limits<unsigned>::max()));
}

TEST(varint, basic_signed) {
  using namespace embedded;

  EXPECT_EQ((8 * sizeof(int) + 6) / 7,
            varint::size(std::numeric_limits<int>::min()));
  EXPECT_EQ(3, varint::size(-8193));
  EXPECT_EQ(2, varint::size(-8192));
  EXPECT_EQ(2, varint::size(-65));
  EXPECT_EQ(1, varint::size(-64));
  EXPECT_EQ(1, varint::size(-1));
  EXPECT_EQ(1, varint::size(0));
  EXPECT_EQ(1, varint::size(1));
  EXPECT_EQ(1, varint::size(63));
  EXPECT_EQ(2, varint::size(64));
  EXPECT_EQ(2, varint::size(8191));
  EXPECT_EQ(3, varint::size(8192));
  EXPECT_EQ((8 * sizeof(int) + 6) / 7,
            varint::size(std::numeric_limits<int>::max()));
}

TEST(varint, encode_decode_error) {
  using namespace embedded;

  std::vector<uint8_t> vec;

  // output buffer exhausted
  EXPECT_TRUE(varint::encode(0U, vec.begin(), vec.end()) == vec.begin());

  vec.resize(1);

  EXPECT_TRUE(varint::encode(0U, vec.begin(), vec.end()) == vec.end());

  // output buffer exhausted
  EXPECT_TRUE(varint::encode(128U, vec.begin(), vec.end()) == vec.begin());

  vec.resize(2);

  EXPECT_TRUE(varint::encode(0U, vec.begin(), vec.end()) == vec.begin() + 1);

  // output buffer exhausted
  EXPECT_TRUE(varint::encode(16384U, vec.begin(), vec.end()) == vec.begin());

  EXPECT_TRUE(varint::encode(255U, vec.begin(), vec.end()) == vec.end());

  uint8_t val8{};

  EXPECT_TRUE(varint::decode(val8, vec.begin(), vec.end()) == vec.end());
  EXPECT_EQ(255, val8);

  EXPECT_TRUE(varint::encode(256U, vec.begin(), vec.end()) == vec.end());

  // target type not wide enough
  EXPECT_TRUE(varint::decode(val8, vec.begin(), vec.end()) == vec.begin());

  uint16_t val16{};

  EXPECT_TRUE(varint::decode(val16, vec.begin(), vec.end()) == vec.end());

  // input buffer exhausted
  EXPECT_TRUE(varint::decode(val16, vec.begin(), vec.begin() + 1) ==
              vec.begin());
}

template <typename T>
class varint_test : public testing::Test {
 public:
  template <typename U, typename std::enable_if<std::is_unsigned<U>::value,
                                                bool>::type = true>
  std::vector<U> get_values() const {
    std::vector<U> values{0, 1, 2, 126, 127, 128, 129, 254};

    if (sizeof(U) >= 2) {
      values.push_back(16383);
      values.push_back(16384);
    }

    if (sizeof(U) >= 4) {
      values.push_back(2097151);
      values.push_back(2097152);
      values.push_back(268435455);
      values.push_back(268435456);
    }

    values.push_back(std::numeric_limits<U>::max());

    return values;
  }

  template <typename U, typename std::enable_if<std::is_signed<U>::value,
                                                bool>::type = true>
  std::vector<U> get_values() const {
    std::vector<U> values{-127, -65, -64, -63, -2, -1, 0,
                          1,    2,   63,  64,  65, 126};

    if (sizeof(U) >= 2) {
      values.push_back(-8193);
      values.push_back(-8192);
      values.push_back(-8191);
      values.push_back(8191);
      values.push_back(8192);
      values.push_back(8193);
    }

    if (sizeof(U) >= 4) {
      values.push_back(-134217729);
      values.push_back(-134217728);
      values.push_back(-134217727);
      values.push_back(134217727);
      values.push_back(134217728);
      values.push_back(134217729);
    }

    values.push_back(std::numeric_limits<U>::min());
    values.push_back(std::numeric_limits<U>::max());

    return values;
  }
};

TYPED_TEST_SUITE_P(varint_test);

TYPED_TEST_P(varint_test, size) {
  using namespace embedded;

  EXPECT_EQ(1, varint::size(TypeParam(0)));
  EXPECT_EQ((8 * sizeof(TypeParam) + 6) / 7,
            varint::size(std::numeric_limits<TypeParam>::max()));
}

TYPED_TEST_P(varint_test, encode_decode) {
  using namespace embedded;

  auto values = this->template get_values<TypeParam>();

  for (auto v : values) {
    std::vector<uint8_t> buf;
    buf.resize(varint::size(v));

    {
      auto it = varint::encode(v, buf.begin(), buf.end());
      EXPECT_TRUE(it == buf.end());
    }

    {
      TypeParam out;
      auto it = varint::decode(out, buf.begin(), buf.end());
      EXPECT_TRUE(it == buf.end());
      EXPECT_EQ(v, out);
    }
  }
}

REGISTER_TYPED_TEST_SUITE_P(varint_test, size, encode_decode);

using UnsignedTypes = ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t,
                                       int8_t, int16_t, int32_t, int64_t>;

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

INSTANTIATE_TYPED_TEST_SUITE_P(varint, varint_test, UnsignedTypes);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
