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

#ifdef DEBUG
#include <iostream>
#endif

#include "tree.hpp"
#include "type_traits.hpp"

namespace sjtu {

namespace internal {

// Resembles __map_value_compare in libc++.
template <typename Key, typename Value, typename Cmp>
class MapValueCompare {
 private:
  using Pair = pair<const Key, Value>;
  Cmp cmp_;
 public:
  auto operator() (const Key &lhs, const Key &rhs) const -> bool {
    return cmp_(lhs, rhs);
  }
  auto operator() (const Key &lhs, const Pair &rhs) const -> bool {
    return cmp_(lhs, rhs.first);
  }
  auto operator() (const Pair &lhs, const Key &rhs) const -> bool {
    return cmp_(lhs.first, rhs);
  }
  auto operator() (const Pair &lhs, const Pair &rhs) const -> bool {
    return cmp_(lhs.first, rhs.first);
  }
};

} // namespace internal

template <typename KeyType, typename ValueType, typename Compare = std::less<KeyType>>
class map {
 public:
  using value_type = pair<const KeyType, ValueType>;
 private:
  using TreeType = typename panic::RbTree<value_type, internal::MapValueCompare<KeyType, ValueType, Compare>>;
 public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  using iterator = typename TreeType::iterator;
  using const_iterator = typename TreeType::const_iterator;

  map () = default;
  /**
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent to key.
   * If no such element exists, an exception of type `index_out_of_bound'
   */
  auto at (const KeyType &key) -> ValueType & {
    auto it = tree_.find(key);
    if (it == tree_.end()) throw index_out_of_bound();
    return it->second;
  }
  auto at (const KeyType &key) const -> const ValueType & {
    auto it = tree_.find(key);
    if (it == tree_.cend()) throw index_out_of_bound();
    return it->second;
  }
  /**
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to key,
   *   performing an insertion if such key does not already exist.
   */
  auto operator[] (const KeyType &key) -> ValueType & {
    // we need to use the default constructor here. Too bad we have no choice.
    auto p = tree_.insert(pair(key, ValueType()));
    return p.first->second;
  }
  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  auto operator[] (const KeyType &key) const -> const ValueType & {
    return at(key);
  }
  /**
   * return a iterator to the beginning
   */
  auto begin () -> iterator {
    return tree_.begin();
  }
  auto cbegin () const -> const_iterator {
    return tree_.cbegin();
  }
  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  auto end () -> iterator {
    return tree_.end();
  }
  auto cend () const -> const_iterator {
    return tree_.cend();
  }
  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  auto empty () const -> bool {
    return tree_.empty();
  }
  /**
   * returns the number of elements.
   */
  auto size () const -> size_t {
    return tree_.size();
  }
  /**
   * clears the contents
   */
  auto clear () -> void {
    tree_.clear();
  }
  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the insertion),
   *   the second one is true if insert successfully, or false.
   */
  auto insert (const value_type &value) -> pair<iterator, bool> {
    return tree_.insert(value);
  }
  /**
   * erase the element at pos.
   * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
   */
  auto erase (iterator pos) -> void {
    return tree_.erase(pos);
  }
  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  auto count (const KeyType &key) const -> size_t {
    auto it = tree_.find(key);
    return it == tree_.cend() ? 0 : 1;
  }
  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is returned.
   */
  auto find (const KeyType &key) -> iterator {
    return tree_.find(key);
  }
  auto find (const KeyType &key) const -> const_iterator {
    return tree_.find(key);
  }

#ifdef DEBUG
  auto print () -> void {
    std::cout << "s=" << size() << " ";
    for (const auto &p : *this) {
      std::cout << "(" << p.first.print() << ", " << p.second.print() << ") ";
    }
    std::cout << std::endl;
  }
#endif

 private:
  TreeType tree_;
};

} // namespace sjtu

#endif // SJTU_MAP_HPP_
