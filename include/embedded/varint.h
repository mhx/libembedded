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

#include <cstdint>
#include <iterator>
#include <type_traits>

namespace embedded {

/**
 * Variable length integer encoding / decoding
 *
 * This class implements the encoding and decoding of integer values to/from
 * a byte stream, using as few bytes as possible, independent of byte-order.
 * The encoding stores the integer values in 7-bit chunks, least signficant
 * chunk first, setting the MSB in all chunks except for the final one. So
 * for example, all values [0, 127] would be stored verbatim as a single byte.
 * A value of 128 would be stored as (128, 1), 16383 as (255, 127).
 *
 * https://developers.google.com/protocol-buffers/docs/encoding#varints
 *
 * In order to store signed values efficiently, these are transformed to
 * unsigned values such that positive numbers are turned into even numbers
 * through multiplication by two and negative numbers are folded into the
 * odd number spots:
 *
 *     0 ->  0      -1 ->  1
 *     1 ->  2      -2 ->  3
 *    17 -> 34     -17 -> 33
 *
 * This transformation and its inverse can be done efficiently and branch-free.
 *
 * https://developers.google.com/protocol-buffers/docs/encoding#signed_integers
 */
class varint {
 public:
  /**
   * Return the number of bytes required to encode an integer value
   */
  template <typename T, typename std::enable_if<std::is_unsigned<T>::value,
                                                bool>::type = true>
  static constexpr std::size_t size(T value) {
    // tail recursion to enable C++11 compliant constexpr
    return value < 128 ? 1 : 1 + size<T>(value >> 7);
  }

  // signed integer version
  template <typename T, typename std::enable_if<std::is_signed<T>::value,
                                                bool>::type = true>
  static constexpr std::size_t size(T value) {
    return size(zig_zag_encode(value));
  }

  /**
   * Encode an integer value
   *
   * \param value    The integer value to encode.
   *
   * \param begin    Iterator to start of encoding buffer.
   *
   * \param end      Iterator to end of encoding buffer.
   *
   * \returns Iterator to end of encoding, or `begin` if there was not enough
   *          space for the encoding. Note that in the error case, the buffer
   *          has been overwritten up until `end`. To avoid this, use `size()`
   *          to check if you have enough space before calling `encode`.
   */
  template <
      typename T, typename It,
      typename std::enable_if<std::is_unsigned<T>::value, bool>::type = true>
  static It encode(T value, It begin, It end) {
    static_assert(std::is_same<typename std::decay<decltype(*begin)>::type,
                               uint8_t>::value,
                  "varint can only be encoded to uint8_t iterators");

    return encode_impl(value, begin, [end](It it) { return it != end; });
  }

  // signed integer version
  template <
      typename T, typename It,
      typename std::enable_if<std::is_signed<T>::value, bool>::type = true>
  static It encode(T value, It begin, It end) {
    return encode(zig_zag_encode(value), begin, end);
  }

  /**
   * Encode an integer value (without range check)
   *
   * \param value    The integer value to encode.
   *
   * \param begin    Iterator to start of encoding buffer.
   *
   * \returns Iterator to end of encoding.
   */
  template <
      typename T, typename It,
      typename std::enable_if<std::is_unsigned<T>::value, bool>::type = true>
  static It encode(T value, It begin) {
    static_assert(std::is_same<typename std::decay<decltype(*begin)>::type,
                               uint8_t>::value,
                  "varint can only be encoded to uint8_t iterators");

    return encode_impl(value, begin, [](It) { return true; });
  }

  // signed integer version
  template <
      typename T, typename It,
      typename std::enable_if<std::is_signed<T>::value, bool>::type = true>
  static It encode(T value, It begin) {
    return encode(zig_zag_encode(value), begin);
  }

  /**
   * Encode an integer value (with back insert iterator)
   *
   * \param value    The integer value to encode.
   *
   * \param begin    Iterator to start of encoding buffer.
   *
   * \returns Iterator to end of encoding.
   */
  template <
      typename T, typename Container,
      typename std::enable_if<std::is_unsigned<T>::value, bool>::type = true>
  static void encode(T value, std::back_insert_iterator<Container> begin) {
    static_assert(
        std::is_same<typename std::decay<typename Container::value_type>::type,
                     uint8_t>::value,
        "varint can only be encoded to uint8_t iterators");

    encode_impl(value, begin,
                [](std::back_insert_iterator<Container>) { return true; });
  }

  // signed integer version
  template <
      typename T, typename Container,
      typename std::enable_if<std::is_signed<T>::value, bool>::type = true>
  static void encode(T value, std::back_insert_iterator<Container> begin) {
    encode(zig_zag_encode(value), begin);
  }

  /**
   * Decode an integer value
   *
   * \param value    Reference to decoded output value.
   *
   * \param begin    Iterator to start of encoding buffer.
   *
   * \param end      Iterator to end of encoding buffer.
   *
   * \returns Iterator to end of encoding, or `begin` if there was an error.
   *          There are two potential error cases. One is where the encoding
   *          continues past the end of the encoding buffer, the other is
   *          where the target type isn't wide enough to store the encoded
   *          value.
   */
  template <
      typename T, typename It,
      typename std::enable_if<std::is_unsigned<T>::value, bool>::type = true>
  static It decode(T& value, It begin, It end) {
    static_assert(std::is_same<typename std::decay<decltype(*begin)>::type,
                               uint8_t>::value,
                  "varint can only be encoded to uint8_t iterators");

    constexpr int type_bits = 8 * sizeof(T);
    constexpr int max_shift = type_bits - 1;
    int shift = 0;
    auto it = begin;

    value = 0;

    for (value = 0; it != end && shift <= max_shift; shift += 7) {
      auto const byte = *it++;

      value |= static_cast<T>(byte & 0x7f) << shift;

      if (!(byte & 0x80)) {
        if (shift > type_bits - 7 && (byte >> (type_bits - shift)) != 0) {
          break;
        }

        return it;
      }
    }

    return begin;
  }

  // signed integer version
  template <
      typename T, typename It,
      typename std::enable_if<std::is_signed<T>::value, bool>::type = true>
  static It decode(T& value, It begin, It end) {
    using utype = typename std::make_unsigned<T>::type;
    utype uvalue;
    auto i = decode(uvalue, begin, end);
    if (i != begin) {
      value = zig_zag_decode(uvalue);
    }
    return i;
  }

  // signed to unsigned transform
  template <
      typename T,
      typename std::enable_if<std::is_signed<T>::value, bool>::type = true,
      typename U = typename std::make_unsigned<T>::type>
  static constexpr U zig_zag_encode(T value) {
    return static_cast<U>((static_cast<U>(value) << 1) ^
                          -(static_cast<U>(value) >> (8 * sizeof(T) - 1)));
  }

  // unsigned to signed transform
  template <
      typename T,
      typename std::enable_if<std::is_unsigned<T>::value, bool>::type = true,
      typename S = typename std::make_signed<T>::type>
  static constexpr S zig_zag_decode(T value) {
    return static_cast<S>((value >> 1) ^ -(value & 1));
  }

 private:
  template <
      typename T, typename It, typename CheckFunc,
      typename std::enable_if<std::is_unsigned<T>::value, bool>::type = true>
  static It encode_impl(T value, It begin, CheckFunc const& check_iter) {
    auto it = begin;

    while (check_iter(it) && value >= 128) {
      *it++ = 0x80 | (value & 0x7f);
      value >>= 7;
    }

    if (check_iter(it)) {
      *it++ = static_cast<uint8_t>(value);
      return it;
    }

    return begin;
  }
};

} // namespace embedded
