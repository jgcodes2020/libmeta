/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This file implements lightweight container types for
  compile-time constants.
*/
#ifndef _M64PP_NTUPLE_HPP_
#define _M64PP_NTUPLE_HPP_

#include <bits/utility.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <meta/fixed_string.hpp>
#include <meta/metatypes.hpp>

namespace meta {
  template <fixed_string K, class T>
  struct nt_arg {
    static constexpr decltype(K) name = K;
    using type                        = T;

    T value;
  };

  namespace details {
    // Concept to detect nt_leaf
    // =========================
    template <class T>
    struct is_any_nt_arg : std::false_type {};

    template <fixed_string S, class T>
    struct is_any_nt_arg<nt_arg<S, T>> : std::true_type {};

    template <class T>
    concept any_nt_arg = is_any_nt_arg<T>::value;
  }  // namespace details

  template <details::any_nt_arg... Ls>
  struct ntuple;

  namespace details {
    // Concept to detect ntuple
    template <class T>
    struct is_any_ntuple : std::false_type {};

    template <details::any_nt_arg... Ls>
    struct is_any_ntuple<ntuple<Ls...>> : std::true_type {};

    template <class T>
    concept any_ntuple = is_any_ntuple<T>::value;

    // Setup to assert uniqueness of strings
    // =====================================

    template <fixed_string... Ss>
    struct string_set : string_constant<Ss>... {
      template <fixed_string S>
      constexpr auto operator+(string_constant<S>) {
        if constexpr (std::is_base_of_v<string_constant<S>, string_set>)
          return string_set {};
        else
          return string_set<Ss..., S> {};
      }

      constexpr size_t size() { return sizeof...(Ss); }
    };

    template <fixed_string... Ss>
    inline constexpr bool strings_unique =
      (string_set<> {} + ... + string_constant<Ss> {}).size() == sizeof...(Ss);

    // Setup for indexing an ntuple
    // ============================

    template <fixed_string K, class T>
    struct nt_empty_leaf {
      using type = T;
    };

    template <class... Ts>
    struct nt_empty_base;

    template <fixed_string... Ks, class... Ts>
    struct nt_empty_base<nt_empty_leaf<Ks, Ts>...> : nt_empty_leaf<Ks, Ts>... {
    };

    template <fixed_string K, class T>
    const nt_empty_leaf<K, T> empty_upcaster(const nt_empty_leaf<K, T>&);

  }  // namespace details

  template <fixed_string K, details::any_ntuple T>
  struct ntuple_element;

  template <fixed_string K, fixed_string... Ks, class... Ts>
  struct ntuple_element<K, ntuple<nt_arg<Ks, Ts>...>> {
    using type = typename decltype(details::empty_upcaster<K>(
      details::nt_empty_base<details::nt_empty_leaf<Ks, Ts>...> {}))::type;
  };

  template <fixed_string K, details::any_ntuple T>
  using ntuple_element_t = typename ntuple_element<K, T>::type;

  /**
   * Actual named tuple class.
   */
  template <fixed_string... Ks, class... Ts>
  class ntuple<nt_arg<Ks, Ts>...> : private nt_arg<Ks, Ts>... {
    static_assert(details::strings_unique<Ks...>, "Keys must be unique");

  public:
    using keys = string_sequence<Ks...>;

    ntuple() = default;
    ntuple(Ts&&... values) : nt_arg<Ks, Ts> {std::forward<Ts>(values)}... {}

    template <fixed_string K, fixed_string... TKs, class... TTs>
    friend ntuple_element_t<K, ntuple<nt_arg<TKs, TTs>...>>& get(
      ntuple<nt_arg<TKs, TTs>...>& tuple);

    template <fixed_string K, fixed_string... TKs, class... TTs>
    friend const ntuple_element_t<K, ntuple<nt_arg<TKs, TTs>...>>& get(
      const ntuple<nt_arg<TKs, TTs>...>& tuple);
  };
  
  // Get by name
  // ==============
  
  template <fixed_string K, fixed_string... Ks, class... Ts>
  ntuple_element_t<K, ntuple<nt_arg<Ks, Ts>...>>& get(
    ntuple<nt_arg<Ks, Ts>...>& tuple) {
    using leaf_type =
      nt_arg<K, ntuple_element_t<K, ntuple<nt_arg<Ks, Ts>...>>>;
    return static_cast<leaf_type&>(tuple).value;
  }

  template <fixed_string K, fixed_string... Ks, class... Ts>
  const ntuple_element_t<K, ntuple<nt_arg<Ks, Ts>...>>& get(
    const ntuple<nt_arg<Ks, Ts>...>& tuple) {
    using leaf_type =
      nt_arg<K, ntuple_element_t<K, ntuple<nt_arg<Ks, Ts>...>>>;
    return static_cast<const leaf_type&>(tuple).value;
  }
  
  template <fixed_string K, fixed_string... Ks, class... Ts>
  ntuple_element_t<K, ntuple<nt_arg<Ks, Ts>...>>&& get(
    ntuple<nt_arg<Ks, Ts>...>&& tuple) {
    using leaf_type =
      nt_arg<K, ntuple_element_t<K, ntuple<nt_arg<Ks, Ts>...>>>;
    return static_cast<leaf_type&>(tuple).value;
  }

  template <fixed_string K, fixed_string... Ks, class... Ts>
  const ntuple_element_t<K, ntuple<nt_arg<Ks, Ts>...>>&& get(
    const ntuple<nt_arg<Ks, Ts>...>&& tuple) {
    using leaf_type =
      nt_arg<K, ntuple_element_t<K, ntuple<nt_arg<Ks, Ts>...>>>;
    return static_cast<const leaf_type&>(tuple).value;
  }
}  // namespace meta

// Structured binding implementation (get by index, size)
// ======================================================

namespace std {
  template <meta::fixed_string... Ks, class... Ts>
  struct tuple_size<meta::ntuple<meta::nt_arg<Ks, Ts>...>> :
    std::integral_constant<size_t, sizeof...(Ks)>{};
  
  template <size_t I, meta::fixed_string... Ks, class... Ts>
  struct tuple_element<I, meta::ntuple<meta::nt_arg<Ks, Ts>...>> {
    using type = meta::ntuple_element_t<
      meta::string_sequence_element<I, meta::string_sequence<Ks...>>::value,
      meta::ntuple<meta::nt_arg<Ks, Ts>...>>;
  };
  
  template <size_t I, meta::fixed_string... Ks, class... Ts>
  tuple_element_t<I, meta::ntuple<meta::nt_arg<Ks, Ts>...>>& get(meta::ntuple<meta::nt_arg<Ks, Ts>...>& tuple) {
    constexpr decltype(auto) key = meta::string_sequence_element_v<I, meta::string_sequence<Ks...>>;
    return meta::get<key>(tuple);
  }
  template <size_t I, meta::fixed_string... Ks, class... Ts>
  const tuple_element_t<I, meta::ntuple<meta::nt_arg<Ks, Ts>...>>& get(const meta::ntuple<meta::nt_arg<Ks, Ts>...>& tuple) {
    constexpr decltype(auto) key = meta::string_sequence_element_v<I, meta::string_sequence<Ks...>>;
    return meta::get<key>(tuple);
  }
  template <size_t I, meta::fixed_string... Ks, class... Ts>
  tuple_element_t<I, meta::ntuple<meta::nt_arg<Ks, Ts>...>>&& get(meta::ntuple<meta::nt_arg<Ks, Ts>...>&& tuple) {
    constexpr decltype(auto) key = meta::string_sequence_element_v<I, meta::string_sequence<Ks...>>;
    return meta::get<key>(tuple);
  }
  template <size_t I, meta::fixed_string... Ks, class... Ts>
  const tuple_element_t<I, meta::ntuple<meta::nt_arg<Ks, Ts>...>>&& get(const meta::ntuple<meta::nt_arg<Ks, Ts>...>&& tuple) {
    constexpr decltype(auto) key = meta::string_sequence_element_v<I, meta::string_sequence<Ks...>>;
    return meta::get<key>(tuple);
  }
}  // namespace std
#endif