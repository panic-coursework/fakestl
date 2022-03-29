/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP_
#define SJTU_MAP_HPP_

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

#include "map.hpp"

namespace sjtu {

namespace internal {

// Resembles __map_value_compare in libc++.
template <typename Key, typename Value, typename Cmp>
class MapValueCompare {
 public:
  auto operator() (const Key &lhs, const Key &rhs) const -> bool {
    return lhs < rhs;
  }
  auto operator() (const Key &lhs, const Pair &rhs) const -> bool {
    return lhs < rhs.first;
  }
  auto operator() (const Pair &lhs, const Key &rhs) const -> bool {
    return lhs.first < rhs;
  }
  auto operator() (const Pair &lhs, const Pair &rhs) const -> bool {
    return lhs.first < rhs.first;
  }
 private:
  using Pair = pair<Key, Value>;
};

} // namespace internal

template <typename KeyType, typename ValueType, typename Compare = std::less<KeyType>>
class map {
 public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  using value_type = pair<const Key, T>;
  using iterator = typename TreeType::iterator;
  using const_iterator = typename TreeType::const_iterator;

  map () {}
  /**
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent to key.
   * If no such element exists, an exception of type `index_out_of_bound'
   */
  auto at (const Key &key) -> T & {}
  auto at (const Key &key) const -> const T & {}
  /**
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to key,
   *   performing an insertion if such key does not already exist.
   */
  auto operator[] (const Key &key) -> T & {}
  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  auto operator[] (const Key &key) const -> const T & {}
  /**
   * return a iterator to the beginning
   */
  auto begin () -> iterator {}
  auto cbegin () const -> const_iterator {}
  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  auto end () -> iterator {}
  auto cend () const -> const_iterator {}
  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  auto empty () const -> bool {}
  /**
   * returns the number of elements.
   */
  auto size () const -> size_t {}
  /**
   * clears the contents
   */
  auto clear () -> void {}
  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the insertion),
   *   the second one is true if insert successfully, or false.
   */
  auto insert (const value_type &value) -> pair<iterator, bool> {}
  /**
   * erase the element at pos.
   * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
   */
  auto erase (iterator pos) -> void {}
  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  auto count (const Key &key) const -> size_t {}
  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is returned.
   */
  auto find (const Key &key) -> iterator {}
  auto find (const Key &key) const -> const_iterator {}
 private:
  using TreeType = panic::RbTree<value_type, internal::MapValueCompare<KeyType, ValueType, Compare>>;
  TreeType tree_;
};

} // namespace sjtu

#endif // SJTU_MAP_HPP_
