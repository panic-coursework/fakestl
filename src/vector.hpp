#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template<typename T>
class vector {
 public:
  /**
   * you can see RandomAccessIterator at CppReference for help.
   */
  class const_iterator;
  class iterator {
  // The following code is written for the C++ type_traits library.
  // Type traits is a C++ feature for describing certain properties of a type.
  // For instance, for an iterator, iterator::value_type is the type that the
  // iterator points to.
  // STL algorithms and containers may use these type_traits (e.g. the following
  // typedef) to work properly. In particular, without the following code,
  // @code{std::sort(iter, iter1);} would not compile.
  // See these websites for more information:
  // https://en.cppreference.com/w/cpp/header/type_traits
  // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
  // About iterator_category: https://en.cppreference.com/w/cpp/iterator
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

   private:
    vector *home_;
    pointer ptr_;
    iterator (vector *home, pointer ptr) : home_(home), ptr_(ptr) {}
   public:
    /**
     * return a new iterator which pointer n-next elements
     * as well as operator-
     */
    auto operator+ (const int &n) const -> iterator {
      return iterator(home_, ptr_ + n);
    }
    auto operator- (const int &n) const -> iterator {
      return iterator(home_, ptr_ - n);
    }
    // return the distance between two iterators,
    // if these two iterators point to different vectors, throw invaild_iterator.
    auto operator- (const iterator &rhs) const -> int {
      if (home_ != rhs.home_) throw invalid_iterator();
      return ptr_ = rhs.ptr_;
    }
    auto operator+= (const int &n) -> iterator & {
      ptr_ += n;
      return *this;
    }
    auto operator-= (const int &n) -> iterator & { return (*this += -n); }
    auto operator++ (int) const -> iterator { return operator+(1); }
    auto operator++ () -> iterator & { return (*this += 1); }
    auto operator-- (int) const -> iterator { return operator+(-1); }
    auto operator-- () -> iterator & { return (*this -= 1); }
    auto operator* () const -> T & { return *ptr_; }
    /**
     * a operator to check whether two iterators are same (pointing to the same memory address).
     */
    auto operator== (const iterator &rhs) const -> bool { return ptr_ == rhs.ptr_; }
    auto operator== (const const_iterator &rhs) const -> bool { return ptr_ == rhs.ptr_; }
    /**
     * some other operator for iterator.
     */
    auto operator!= (const iterator &rhs) const -> bool { return !(*this == rhs); }
    auto operator!= (const const_iterator &rhs) const -> bool { return !(*this == rhs); }
    friend class const_iterator;
    friend class vector;
  };
  class const_iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

   private:
    const vector *home_;
    const T *ptr_;
    const_iterator (const vector *home, pointer ptr) : home_(home), ptr_(ptr) {}
   public:
    /**
     * return a new iterator which pointer n-next elements
     * as well as operator-
     */
    auto operator+ (const int &n) const -> const_iterator {
      return const_iterator(home_, ptr_ + n);
    }
    auto operator- (const int &n) const -> const_iterator {
      return const_iterator(home_, ptr_ - n);
    }
    auto operator- (const const_iterator &rhs) const -> int {
      if (home_ != rhs.home_) throw invalid_iterator();
      return ptr_ = rhs.ptr_;
    }
    auto operator+= (const int &n) -> const_iterator & {
      ptr_ += n;
      return *this;
    }
    auto operator-= (const int &n) -> const_iterator & { return (*this += -n); }
    auto operator++ (int) const -> const_iterator { return operator+(1); }
    auto operator++ () -> const_iterator & { return (*this += 1); }
    auto operator-- (int) const -> const_iterator { return operator+(-1); }
    auto operator-- () -> const_iterator & { return (*this -= 1); }
    auto operator* () const -> const T & { return *ptr_; }
    /**
     * a operator to check whether two iterators are same (pointing to the same memory address).
     */
    auto operator== (const iterator &rhs) const -> bool { return ptr_ == rhs.ptr_; }
    auto operator== (const const_iterator &rhs) const -> bool { return ptr_ == rhs.ptr_; }
    /**
     * some other operator for iterator.
     */
    auto operator!= (const iterator &rhs) const -> bool { return !(*this == rhs); }
    auto operator!= (const const_iterator &rhs) const -> bool { return !(*this == rhs); }
    friend class iterator;
    friend class vector;
  };
  /**
   * TODO Constructs
   * Atleast two: default constructor, copy constructor
   */
  vector () = default;
  vector (const vector &other) { *this = other; }
  /**
   * TODO Destructor
   */
  ~vector () {
    destroyContents_();
    delete[] reinterpret_cast<char *>(storage_);
  }
  /**
   * TODO Assignment operator
   */
  auto operator= (const vector &other) -> vector & {
    if (this == &other) return *this;
    clear();
    grow_(other.capacity_);
    size_ = other.size_;
    copyContents_(storage_, other.storage_, size_);
    return *this;
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   */
  auto at (const size_t &pos) -> T & {
    checkPosition_(pos);
    return storage_[pos];
  }
  auto at (const size_t &pos) const -> const T & {
    return const_cast<vector *>(this)->at(pos);
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   * !!! Pay attentions
   *   In STL this operator does not check the boundary but I want you to do.
   */
  auto operator[] (const size_t &pos) -> T & { return at(pos); }
  auto operator[] (const size_t &pos) const -> const T & { return at(pos); }
  /**
   * access the first element.
   * throw container_is_empty if size == 0
   */
  auto front () const -> const T & {
    checkNonEmpty_();
    return at(0);
  }
  /**
   * access the last element.
   * throw container_is_empty if size == 0
   */
  auto back () const -> const T & {
    checkNonEmpty_();
    return at(size_ - 1);
  }
  /**
   * returns an iterator to the beginning.
   */
  auto begin () -> iterator {
    return iterator(this, storage_);
  }
  auto cbegin () const -> const_iterator {
    return const_iterator(this, storage_);
  }
  /**
   * returns an iterator to the end.
   */
  auto end () -> iterator {
    return iterator(this, storage_ + size_);
  }
  auto cend () const -> const_iterator {
    return const_iterator(this, storage_ + size_);
  }
  /**
   * checks whether the container is empty
   */
  auto empty () const -> bool {
    return size_ == 0;
  }
  /**
   * returns the number of elements
   */
  auto size () const -> size_t {
    return size_;
  }
  /**
   * clears the contents
   */
  auto clear () -> void {
    destroyContents_();
    delete[] reinterpret_cast<char *>(storage_);
    storage_ = nullptr;
    capacity_ = 0;
    size_ = 0;
  }
  /**
   * inserts value before pos
   * returns an iterator pointing to the inserted value.
   */
  auto insert (iterator pos, const T &value) -> iterator { return insert(pos.ptr_ - storage_, value); }
  /**
   * inserts value at index ind.
   * after inserting, this->at(ind) == value
   * returns an iterator pointing to the inserted value.
   * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
   */
  auto insert (const size_t &ix, const T &value) -> iterator {
    if (ix > size_) throw index_out_of_bound();
    if (size_ == capacity_) grow_();
    for (size_t i = size_; i > ix; --i) {
      storage_[i] = move_(storage_[i - 1]);
    }
    storage_[ix] = value;
    ++size_;
    return iterator(this, storage_ + ix);
  }
  /**
   * removes the element at pos.
   * return an iterator pointing to the following element.
   * If the iterator pos refers the last element, the end() iterator is returned.
   */
  auto erase (iterator pos) -> iterator { return erase(pos.ptr_ - storage_); }
  /**
   * removes the element with index ind.
   * return an iterator pointing to the following element.
   * throw index_out_of_bound if ind >= size
   */
  auto erase (const size_t &ix) -> iterator {
    checkPosition_(ix);
    for (size_t i = ix; i + 1 < size_; ++i) {
      storage_[i] = move_(storage_[i + 1]);
    }
    (storage_ + size_ - 1)->~T();
    --size_;
    return iterator(this, storage_ + ix);
  }
  /**
   * adds an element to the end.
   */
  auto push_back (const T &value) -> void {
    if (size_ == capacity_) grow_();
    new(storage_ + size_) T(value);
    ++size_;
  }
  /**
   * remove the last element from the end.
   * throw container_is_empty if size() == 0
   */
  auto pop_back () -> void {
    checkNonEmpty_();
    (storage_ + size_ - 1)->~T();
    --size_;
  }

 private:
  static constexpr size_t kSzDefault_ = 4;
  static constexpr size_t kSzT_ = sizeof(T);
  T *storage_ = nullptr;
  size_t capacity_ = 0;
  size_t size_ = 0;

  static auto move_ (T &el) -> T && { return reinterpret_cast<T &&>(el); }
  static auto copyContents_ (T *to, T *from, size_t n) -> void {
    for (size_t i = 0; i < n; ++i) {
      to[i] = from[i];
    }
  }
  static auto moveContents_ (T *to, T *from, size_t n) -> void {
    for (size_t i = 0; i < n; ++i) {
      new(to + i) T(move_(from[i]));
      from[i].~T();
    }
  }
  static auto destroyContents_ (T *array, size_t n) -> void {
    for (size_t i = 0; i < n; ++i) {
      (array + i)->~T();
    }
  }
  auto destroyContents_ () -> void { destroyContents_(storage_, size_); }
  auto grow_ (size_t capNew) -> void {
    T *storeNew = reinterpret_cast<T *>(new char[capNew * kSzT_]);
    if (storage_ != nullptr) {
      moveContents_(storeNew, storage_, size_);
      delete[] reinterpret_cast<char *>(storage_);
    }
    storage_ = storeNew;
    capacity_ = capNew;
  }
  auto grow_ () -> void {
    grow_(storage_ == nullptr ? kSzDefault_ : 2 * capacity_);
  }
  auto checkPosition_ (size_t pos) const -> void {
    // since this is size_t which is unsigned, we could not have pos < 0.
    if (pos >= size_) throw index_out_of_bound();
  }
  auto checkNonEmpty_ () const -> void {
    if (size_ == 0) throw container_is_empty();
  }
};

} // namespace sjtu

#endif
