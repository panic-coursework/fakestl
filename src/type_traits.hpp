#ifndef SJTU_TYPE_TRAITS_HPP_
#define SJTU_TYPE_TRAITS_HPP_

namespace sjtu {

template <typename T, T v>
class integral_constant {
 public:
  static constexpr const T value = v;
  using value_type = T;
  using type = integral_constant;
  constexpr auto operator() () const noexcept -> T {
    return v;
  }
  constexpr operator value_type () const noexcept {
    return v;
  }
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

using my_true_type = true_type;
using my_false_type = false_type;

template <typename>
class is_void : public false_type {};
template <>
class is_void<void> : public true_type {};
template <typename T>
constexpr bool is_void_v = is_void<T>::value;

template <typename T>
class is_const : public false_type {};
template <typename T>
class is_const<const T> : public true_type {};
template <typename T>
constexpr bool is_const_v = is_const<T>::value;

namespace internal {

template <typename T1, typename T2>
class TypePair {
 public:
  using first = T1;
  using second = T2;
};

template <typename T>
auto assignable (int) -> typename TypePair<decltype((std::declval<T>() = std::declval<T>())), true_type>::second;
template <typename T>
auto assignable (...) -> false_type;

template <typename T>
using type_not = integral_constant<bool, !T::value>;

} // namespace internal

template <typename T>
class self_assignable : public integral_constant<
  bool,
  decltype(internal::assignable<T>(0))::value && !is_void_v<T>
> {};

template <typename T>
class iterator_traits {
 public:
  using value_type = typename T::value_type;
  using difference_type = typename T::difference_type;
  using pointer = typename T::pointer;
  using reference = typename T::reference;
  using iterator_category = typename T::iterator_category;
  // using iterator_assignable = self_assignable<value_type>;
  using iterator_assignable = typename internal::type_not<is_const<value_type>>;
};

template <typename T>
using my_type_traits = iterator_traits<T>;

} // namespace sjtu

#endif // SJTU_TYPE_TRAITS_HPP_
