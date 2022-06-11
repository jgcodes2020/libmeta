#ifndef META_CTNPRINT_HPP_
#define META_CTNPRINT_HPP_

#include <bits/utility.h>
#include <istream>
#include <ostream>
#include <syncstream>
#include <type_traits>

#include <meta/metatypes.hpp>
#include <meta/named_tuple.hpp>

namespace meta {
  namespace details {
    // detect non-string containers
    template <class T, class = void>
    struct detect_non_string_range : std::bool_constant<std::ranges::range<T>>{};
    
    template <class T>
    struct detect_non_string_range<T, std::void_t<typename T::traits_type>> : std::false_type {};
    
    template <class T>
    concept non_string_range = detect_non_string_range<T>::value;
  }  // namespace details

  template <class C, class CT>
  std::basic_ostream<C, CT>& operator<<(
    std::basic_ostream<C, CT>& out,
    const details::non_string_range auto& range) {
    namespace ranges = std::ranges;

    auto it        = ranges::cbegin(range);
    const auto end = ranges::cend(range);

    {
      std::basic_osyncstream<C, CT> oss(out);
      oss << '{';
      if (it != end)
        oss << *it++;
      while (it != end) {
        oss << ", " << *it++;
      }
      oss << '}';
    }
    return out;
  }

  namespace details {
    template <size_t B, class T>
    struct offset_index;

    template <size_t B, size_t... Is>
    struct offset_index<B, std::index_sequence<Is...>> {
      using type = std::index_sequence<(Is + B)...>;
    };

    template <size_t B, size_t E>
    using make_index_range =
      typename offset_index<B, std::make_index_sequence<E - B>>::type;
  }  // namespace details

  template <class C, class CT, class... Ts>
  std::basic_ostream<C, CT>& operator<<(
    std::basic_ostream<C, CT>& out, const std::tuple<Ts...>& t) {
    constexpr size_t len = sizeof...(Ts);
    {
      std::basic_osyncstream<C, CT> oss(out);
      oss << '(';
      if constexpr (len >= 1)
        oss << std::get<0>(t);
      if constexpr (len >= 2) {
        using seq_t = details::make_index_range<1, len>;
        [&]<size_t... Is>(std::index_sequence<Is...>) {
          (void(oss << ", " << std::get<Is>(t)), ...);
        }
        (seq_t {});
      }
      oss << ')';
    }
    return out;
  }

  namespace details {
    template <fixed_string... Ss>
    struct splice_first_string;

    template <fixed_string S0, fixed_string... SR>
    struct splice_first_string<S0, SR...> {
      static constexpr decltype(S0) first = S0;
      using remaining                     = string_sequence<SR...>;
    };
  }  // namespace details

  template <class C, class CT, fixed_string... Ks, class... Ts>
  std::basic_ostream<C, CT>& operator<<(
    std::basic_ostream<C, CT>& out,
    const meta::ntuple<meta::nt_arg<Ks, Ts>...>& t) {
    constexpr size_t len = sizeof...(Ts);
    {
      std::basic_osyncstream<C, CT> oss(out);
      oss << '(';
      if constexpr (len >= 1) {
        constexpr fixed_string first =
          details::splice_first_string<Ks...>::first;
        using rem = typename details::splice_first_string<Ks...>::remaining;

        oss << first << " = " << get<first>(t);
        if constexpr (len >= 2) {
          [&]<fixed_string... Ss>(string_sequence<Ss...>) {
            (void(oss << "; " << Ss << " = " << get<Ss>(t)), ...);
          }
          (rem {});
        }
      }
      oss << ')';
    }
    return out;
  }
}  // namespace meta

#endif