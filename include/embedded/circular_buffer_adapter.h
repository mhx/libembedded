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

#include <cassert>
#include <cstring>

#include "config.h"

#if LIBEMB_HAS_EXCEPTIONS
#include <stdexcept>
#endif

#include "detail/circular_buffer_adapter.h"

namespace embedded {

/**
 * An adapter to use arbitrary memory as a circular buffer
 *
 * This adapter allows you to access an arbitrary piece of memory, for example
 * a section of a memory-mapped EEPROM, as a circular buffer.
 *
 * The adapter interface is mostly compatible with std::deque, although it
 * currenly lacks operations such as `insert` and `erase`.
 *
 * While the adapter can be used to implement a circular buffer for non-trivial
 * types, care must be taken to ensure object lifetime is handled properly.
 * If the adapter is initialized to a non-empty circular buffer (i.e. non-zero
 * `item_count`) of non-trivial types, it assumes that the memory passed in
 * contains valid instances in the right slots. Similarly, the adapter won't
 * dispose of any instances that still exist in the managed memory when it is
 * itself destroyed. It will, however, properly manage objects of non-trivial
 * type during its lifetime.
 */
template <typename T>
class circular_buffer_adapter {
 public:
  using value_type = typename std::remove_const<T>::type;
  using reference = T&;
  using const_reference = value_type const&;
  using pointer = T*;
  using const_pointer = value_type const*;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using iterator =
      detail::cba_iterator<T, detail::cba_mutable_iterator_traits<T>>;
  using const_iterator =
      detail::cba_iterator<T, detail::cba_const_iterator_traits<T>>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  friend iterator;
  friend const_iterator;

  circular_buffer_adapter() = default;

  circular_buffer_adapter(circular_buffer_adapter&&) = default;
  circular_buffer_adapter& operator=(circular_buffer_adapter&&) = default;

  circular_buffer_adapter(pointer data, size_type capacity)
      : circular_buffer_adapter(data, capacity, 0, 0) {}

  circular_buffer_adapter(pointer data, size_type capacity,
                          size_type first_index, size_type item_count)
      : circular_buffer_adapter(data, data + capacity, data + first_index,
                                item_count) {}

  circular_buffer_adapter(pointer begin, pointer end)
      : circular_buffer_adapter(begin, end, begin, 0) {}

  circular_buffer_adapter(pointer begin, pointer end, pointer first,
                          size_type item_count)
      : begin_{begin}
      , end_{end}
      , first_{first}
      , last_{add(first, item_count)}
      , size_{item_count} {
    assert(begin <= first);
    assert(first <= end);
    assert(size_ <= capacity());
  }

  iterator begin() const { return iterator(this, first_iter()); }
  iterator end() const { return iterator(this, nullptr); }

  const_iterator cbegin() const { return const_iterator(this, first_iter()); }
  const_iterator cend() const { return const_iterator(this, nullptr); }

  reverse_iterator rbegin() const { return reverse_iterator(end()); }
  reverse_iterator rend() const { return reverse_iterator(begin()); }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }

  void clear() {
    destroy<T>(first_, size_);
    first_ = last_ = begin_;
    size_ = 0;
  }

  bool empty() const { return size_ == 0; }
  bool full() const { return size_ == capacity(); }

  size_type capacity() const { return std::distance(begin_, end_); }
  size_type size() const { return size_; }
  size_type remaining() const { return capacity() - size_; }

  reference front() const {
    assert(!empty());
    return *first_;
  }

  reference back() const {
    assert(!empty());
    return *((last_ != begin_ ? last_ : end_) - 1);
  }

  reference operator[](size_type pos) const {
    assert(pos < size());
    return *add(first_, pos);
  }

  reference at(size_type pos) const {
#if LIBEMB_HAS_EXCEPTIONS
    if (pos >= size()) {
      throw std::out_of_range("circular_buffer_adapter");
    }
#endif
    return (*this)[pos];
  }

  void push_front(const value_type& value) {
    assert(!full());
    new (prev(first_)) value_type(value);
    dec(first_);
    ++size_;
  }

  void push_front(value_type&& value) {
    assert(!full());
    new (prev(first_)) value_type(std::move(value));
    dec(first_);
    ++size_;
  }

  template <typename... Args>
  reference emplace_front(Args&&... args) {
    assert(!full());
    auto p = new (prev(first_)) value_type(std::forward<Args>(args)...);
    dec(first_);
    ++size_;
    return *p;
  }

  void pop_front() {
    assert(!empty());
    destroy(first_);
    inc(first_);
    --size_;
  }

  void pop_front(size_type count) {
    assert(count <= size());
    destroy<T>(first_, count);
    first_ = add(first_, count);
    size_ -= count;
  }

  void push_back(const value_type& value) {
    assert(!full());
    new (last_) value_type(value);
    inc(last_);
    ++size_;
  }

  void push_back(value_type&& value) {
    assert(!full());
    new (last_) value_type(std::move(value));
    inc(last_);
    ++size_;
  }

  template <typename... Args>
  reference emplace_back(Args&&... args) {
    assert(!full());
    auto p = new (last_) value_type(std::forward<Args>(args)...);
    inc(last_);
    ++size_;
    return *p;
  }

  void pop_back() {
    assert(!empty());
    dec(last_);
    destroy(last_);
    --size_;
  }

  void pop_back(size_type count) {
    assert(count <= size());
    last_ = sub(last_, count);
    destroy<T>(last_, count);
    size_ -= count;
  }

  template <typename Traits>
  size_type raw_index(detail::cba_iterator<T, Traits> const& it) const {
    return std::distance(begin_, it.realiter());
  }

  template <typename U,
            typename std::enable_if<std::is_trivial<U>::value &&
                                        std::is_same<U, value_type>::value,
                                    bool>::type = true>
  void copy_in_front(U const* data, size_type count) {
    assert(count <= remaining());
    auto new_first = sub(first_, count);
    copy_in(new_first, data, count);
    size_ += count;
    first_ = new_first;
  }

  template <typename U,
            typename std::enable_if<std::is_trivial<U>::value &&
                                        std::is_same<U, value_type>::value,
                                    bool>::type = true>
  void copy_in_back(U const* data, size_type count) {
    assert(count <= remaining());
    copy_in(last_, data, count);
    last_ = add(last_, count);
    size_ += count;
  }

  template <typename U,
            typename std::enable_if<std::is_trivial<U>::value &&
                                        std::is_same<U, value_type>::value,
                                    bool>::type = true>
  void copy_out_front(U* data, size_type count) {
    assert(count <= size());
    copy_out(data, first_, count);
    first_ = add(first_, count);
    size_ -= count;
  }

  template <typename U,
            typename std::enable_if<std::is_trivial<U>::value &&
                                        std::is_same<U, value_type>::value,
                                    bool>::type = true>
  void copy_out_back(U* data, size_type count) {
    assert(count <= size());
    auto new_last = sub(last_, count);
    copy_out(data, new_last, count);
    size_ -= count;
    last_ = new_last;
  }

  template <typename U,
            typename std::enable_if<std::is_trivial<U>::value &&
                                        std::is_same<U, value_type>::value,
                                    bool>::type = true>
  void copy_in(iterator first, iterator last, U const* data) {
    assert(begin() <= first);
    assert(first <= last);
    assert(last <= end());
    copy_in(first.it_, data, last - first);
  }

  template <typename U,
            typename std::enable_if<std::is_trivial<U>::value &&
                                        std::is_same<U, value_type>::value,
                                    bool>::type = true>
  void copy_out(const_iterator first, const_iterator last, U* data) const {
    assert(begin() <= first);
    assert(first <= last);
    assert(last <= end());
    copy_out(data, first.it_, last - first);
  }

 private:
  template <typename Func>
  void copy_in_impl(pointer dest, value_type const* src, size_type count,
                    Func const& copy_fn) {
    if (dest + count <= end_) {
      copy_fn(dest, src, count);
    } else {
      size_type const count_a = std::distance(dest, end_);
      size_type const count_b = count - count_a;
      copy_fn(dest, src, count_a);
      copy_fn(begin_, src + count_a, count_b);
    }
  }

  template <typename Func>
  void copy_out_impl(value_type* dest, const_pointer src, size_type count,
                     Func const& copy_fn) const {
    if (src + count <= end_) {
      copy_fn(dest, src, count);
    } else {
      size_type const count_a =
          std::distance(src, const_cast<const_pointer>(end_));
      size_type const count_b = count - count_a;
      copy_fn(dest, src, count_a);
      copy_fn(dest + count_a, begin_, count_b);
    }
  }

  void copy_in(pointer dest, value_type const* src, size_type count) {
    copy_in_impl(dest, src, count,
                 [](pointer dest, value_type const* src, size_type count) {
                   std::memcpy(dest, src, sizeof(*dest) * count);
                 });
  }

  void copy_out(value_type* dest, const_pointer src, size_type count) const {
    copy_out_impl(dest, src, count,
                  [](value_type* dest, const_pointer src, size_type count) {
                    std::memcpy(dest, src, sizeof(*dest) * count);
                  });
  }

  template <typename U, typename std::enable_if<std::is_trivial<U>::value,
                                                bool>::type = true>
  void destroy(pointer, size_type) {}

  template <typename U, typename std::enable_if<!std::is_trivial<U>::value,
                                                bool>::type = true>
  void destroy(pointer first, size_type count) {
    while (count > 0) {
      destroy(first);
      inc(first);
      --count;
    }
  }

  void destroy(value_type* p) const { p->~value_type(); }

  pointer first_iter() const { return size_ != 0 ? first_ : 0; }

  void inc(pointer& p) const {
    assert(p != nullptr);
    if (++p == end_) {
      p = begin_;
    }
  }

  void dec(pointer& p) const {
    assert(p != nullptr);
    if (p == begin_) {
      p = end_;
    }
    --p;
  }

  pointer prev(pointer p) const {
    dec(p);
    return p;
  }

  difference_type wrap_around(difference_type n) const {
    return n - static_cast<difference_type>(capacity());
  }

  pointer add(pointer p, difference_type n) const {
    assert(p != nullptr);
    assert(n >= 0);
    return p + (n < std::distance(p, end_) ? n : wrap_around(n));
  }

  pointer sub(pointer p, difference_type n) const {
    assert(p != nullptr);
    assert(n >= 0);
    return p - (n <= std::distance(begin_, p) ? n : wrap_around(n));
  }

  difference_type index(const_pointer cp) const {
    assert(cp != nullptr);
    auto p = const_cast<pointer>(cp);
    return p < first_ ? std::distance(first_, end_) + std::distance(begin_, p)
                      : std::distance(first_, p);
  }

  pointer begin_{nullptr};
  pointer end_{nullptr};
  pointer first_{nullptr};
  pointer last_{nullptr};
  size_type size_{0};
};

} // namespace embedded
