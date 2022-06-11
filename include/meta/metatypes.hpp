/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This file implements lightweight wrapper types for
  compile-time constants.
*/
#ifndef META_METATYPES_HPP
#define META_METATYPES_HPP

#include <bits/utility.h>
#include <cstddef>
#include <string>
#include <type_traits>
#include <meta/fixed_string.hpp>

namespace meta {
  namespace details {
    template <class T>
    concept has_char_traits = requires {
      std::char_traits<T> {};
    };
  };  // namespace details

  template <class T>
  using type_constant = std::type_identity<T>;

  template <class... Ts>
  struct type_sequence {
    static constexpr size_t size = sizeof...(Ts);
  };

  template <fixed_string S>
  using string_constant = std::integral_constant<decltype(S), S>;
  template <fixed_wstring S>
  using wstring_constant = std::integral_constant<decltype(S), S>;
  template <fixed_u16string S>
  using u16string_constant = std::integral_constant<decltype(S), S>;
  template <fixed_u32string S>
  using u32string_constant = std::integral_constant<decltype(S), S>;
  template <fixed_u8string S>
  using u8string_constant = std::integral_constant<decltype(S), S>;

  template <template <size_t> class T, T... Ss>
  struct basic_string_sequence {
    static constexpr size_t size() { return sizeof...(Ss); }
  };

  template <fixed_string... Ss>
  using string_sequence = basic_string_sequence<fixed_string, Ss...>;
  template <fixed_wstring... Ss>
  using wstring_sequence = basic_string_sequence<fixed_wstring, Ss...>;
  template <fixed_u16string... Ss>
  using u16string_sequence = basic_string_sequence<fixed_u16string, Ss...>;
  template <fixed_u32string... Ss>
  using u32string_sequence = basic_string_sequence<fixed_u32string, Ss...>;
  template <fixed_u8string... Ss>
  using u8string_sequence = basic_string_sequence<fixed_u8string, Ss...>;

  namespace details {
    template <template <size_t> class T, T S, size_t I>
    struct basic_string_indexer_leaf {
      static constexpr decltype(S) value = S;
    };

    template <class IS, class SS>
    struct basic_string_indexer_root;

    template <size_t... Is, template <size_t> class T, T... Ss>
    struct basic_string_indexer_root<
      std::index_sequence<Is...>, basic_string_sequence<T, Ss...>>
      : basic_string_indexer_leaf<T, Ss, Is>... {};

    template <size_t I, template <size_t> class T, T S>
    basic_string_indexer_leaf<T, S, I> basic_string_indexer_upcast(
      basic_string_indexer_leaf<T, S, I>&&);
  }  // namespace details

  template <size_t I, class SS>
  struct string_sequence_element;

  template <size_t I, template <size_t> class T, T... Ss>
  struct string_sequence_element<I, basic_string_sequence<T, Ss...>> {
    static constexpr decltype(auto) value =
      decltype(details::basic_string_indexer_upcast<I>(
        details::basic_string_indexer_root<
          std::make_index_sequence<sizeof...(Ss)>,
          basic_string_sequence<T, Ss...>> {}))::value;
  };
  
  template <size_t I, class SS>
  inline constexpr decltype(auto) string_sequence_element_v = string_sequence_element<I, SS>::value;
}  // namespace meta

#endif