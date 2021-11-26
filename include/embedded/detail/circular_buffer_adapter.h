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
#include <iterator>
#include <type_traits>

namespace embedded {

template <typename T>
class circular_buffer_adapter;

namespace detail {

template <typename T>
struct cba_const_iterator_traits {
  using value_type = typename std::remove_const<T>::type;
  using pointer = T const*;
  using reference = T const&;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
};

template <typename T>
struct cba_mutable_iterator_traits {
  using value_type = typename std::remove_const<T>::type;
  using pointer = T*;
  using reference = T&;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
};

template <typename T, typename Traits>
class cba_iterator {
 public:
  friend class circular_buffer_adapter<T>;
  friend class cba_iterator<T, cba_const_iterator_traits<T>>;
  friend class cba_iterator<T, cba_mutable_iterator_traits<T>>;

  using iterator_category = std::random_access_iterator_tag;
  using value_type = typename Traits::value_type;
  using reference = typename Traits::reference;
  using pointer = typename Traits::pointer;
  using size_type = typename Traits::size_type;
  using difference_type = typename Traits::difference_type;

  cba_iterator() = default;

  cba_iterator(cba_iterator<T, cba_mutable_iterator_traits<T>> const& other)
      : adapter_{other.adapter_}
      , it_{other.it_} {}

  cba_iterator(cba_iterator&&) = default;
  cba_iterator& operator=(cba_iterator const&) = default;
  cba_iterator& operator=(cba_iterator&&) = default;

  pointer operator->() const { return it_; }

  reference operator*() const {
    assert(it_ != nullptr);
    return *it_;
  }

  reference operator[](difference_type n) const {
    assert(index() + n < static_cast<difference_type>(adapter_->size()));
    return *(*this + n);
  }

  template <typename Tr>
  bool operator==(const cba_iterator<T, Tr>& other) const {
    return it_ == other.it_;
  }

  template <typename Tr>
  bool operator!=(const cba_iterator<T, Tr>& other) const {
    return !(*this == other);
  }

  template <typename Tr>
  bool operator<(const cba_iterator<T, Tr>& other) const {
    return index() < other.index();
  }

  template <typename Tr>
  bool operator>(const cba_iterator<T, Tr>& other) const {
    return other < *this;
  }

  template <typename Tr>
  bool operator<=(const cba_iterator<T, Tr>& other) const {
    return !(other < *this);
  }

  template <typename Tr>
  bool operator>=(const cba_iterator<T, Tr>& other) const {
    return !(*this < other);
  }

  cba_iterator& operator++() {
    assert(index() < static_cast<difference_type>(adapter_->size()));
    adapter_->inc(it_);
    if (it_ == adapter_->last_) {
      it_ = nullptr;
    }
    return *this;
  }

  cba_iterator& operator--() {
    assert(index() > 0);
    if (it_ == 0) {
      it_ = adapter_->last_;
    }
    adapter_->dec(it_);
    return *this;
  }

  cba_iterator operator++(int) {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  cba_iterator operator--(int) {
    auto tmp = *this;
    --*this;
    return tmp;
  }

  cba_iterator& operator+=(difference_type n) {
    if (n > 0) {
      assert(index() + n <= static_cast<difference_type>(adapter_->size()));
      it_ = adapter_->add(it_, n);
      if (it_ == adapter_->last_) {
        it_ = nullptr;
      }
    } else if (n < 0) {
      *this -= -n;
    }
    return *this;
  }

  cba_iterator& operator-=(difference_type n) {
    if (n > 0) {
      assert(n <= index());
      it_ = adapter_->sub(it_ ? it_ : adapter_->last_, n);
    } else if (n < 0) {
      *this += -n;
    }
    return *this;
  }

  cba_iterator operator+(difference_type n) const {
    return cba_iterator(*this) += n;
  }

  cba_iterator operator-(difference_type n) const {
    return cba_iterator(*this) -= n;
  }

  difference_type operator-(cba_iterator const& other) const {
    return index() - other.index();
  }

 private:
  cba_iterator(circular_buffer_adapter<T> const* adapter, pointer it)
      : adapter_{adapter}
      , it_{it} {}

  difference_type index() const {
    return it_ ? adapter_->index(it_) : adapter_->size();
  }

  circular_buffer_adapter<T> const* adapter_{nullptr};
  pointer it_{nullptr};
};

template <typename T, typename Traits>
cba_iterator<T, Traits> operator+(typename Traits::difference_type n,
                                  cba_iterator<T, Traits> const& iter) {
  return iter + n;
}

} // namespace detail

} // namespace embedded
