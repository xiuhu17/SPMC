#include<iostream>
#include <functional>
#include <tuple>
#include <type_traits>


template<size_t n>
using index_constant = std::integral_constant<size_t, n>;

template<class ... Args>
class binder_list {
  public:
    template<class ... TArgs>
    constexpr binder_list(TArgs&&... args) noexcept: boundedArgs_{std::forward<TArgs>(args)...} {}
    template<size_t n>
    constexpr decltype(auto) operator[](index_constant<n>) noexcept {
      return std::get<n>(boundedArgs_);
    }
  private:
    std::tuple<Args...> boundedArgs_;
  };

template<class ... Args>
class callee_list {
  public:
    template<class ... TArgs>
    constexpr callee_list(TArgs&&... args) noexcept: boundedArgs_{std::forward<TArgs>(args)...} {}
    template<class T, std::enable_if_t<(std::is_placeholder<std::remove_reference_t<T>>::value == 0)>* = nullptr>
    constexpr decltype(auto) operator[](T&& t) noexcept {
      return std::forward<T>(t);
    }

    template<class T, std::enable_if_t<(std::is_placeholder<T>::value != 0)>* = nullptr>
    constexpr decltype(auto) operator[](T) noexcept {
      return std::get<std::is_placeholder<T>::value - 1>(std::move(boundedArgs_));
    }
  private:
    std::tuple<Args&&...> boundedArgs_;
};

template<class Fn, class ... Args>
class binder {
  public:
    template<class TFn, class ... TArgs>
    constexpr binder(TFn&& f, TArgs&&... args) noexcept: f_{std::forward<TFn>(f)}, argumentList_{std::forward<TArgs>(args)...} {}
    // Please C++, give me a way of detecting noexcept :'(
    template<class ... CallArgs>
    constexpr decltype(auto) operator()(CallArgs&&... args) //noexcept(noexcept(call(std::make_index_sequence<sizeof...(Args)>{}, std::declval<Args>()...))) {
      return call(std::make_index_sequence<sizeof...(Args)>{}, std::forward<CallArgs>(args)...);
    }
  private:
    template<class ... CallArgs, size_t ... Seq>
    constexpr decltype(auto) call(std::index_sequence<Seq...>, CallArgs&&... args) //noexcept(noexcept(f_(this->binder_list<CallArgs...>{std::declval<CallArgs>()...}[this-
