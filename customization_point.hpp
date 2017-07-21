#pragma once

#include "multi_function.hpp"
#include "invoke.hpp"
#include <utility>

// Inspired by
// * Eric Niebler's suggested design for customization points and
// * Cris Cecka's design for customizing customization points
// * Jared Hoberock's design for multi_functions and customization points

namespace experimental
{
namespace detail
{


// this function, used as a template parameter to multi_function below,
// converts a call to a customization point object:
//
//     customization_point(customizer, args...)
//
// into a call to experimental::invoke:
//
//     invoke(customizer, customization_point, args...)
//
// The customization point itself as passed as the first parameter,
// followed by the Customizer used as the first parameter to the customization_point call,
// followed by the rest of the arguments to the customization_point call.
struct invoke_customization_point
{
  template<class CustomizationPoint, class Customizer, class... Args>
  constexpr auto operator()(CustomizationPoint&& self, Customizer&& customizer, Args&&... args) const ->
    decltype(experimental::invoke(std::forward<Customizer>(customizer), std::forward<CustomizationPoint>(self), std::forward<Args>(args)...))
  {
    return experimental::invoke(std::forward<Customizer>(customizer), std::forward<CustomizationPoint>(self), std::forward<Args>(args)...);
  }
};


// this functor wraps another Function
// when drop_first_arg_and_invoke is called,
// it ignores its first argument and calls the Function with the remaining arguments
template<class Function>
struct drop_first_arg_and_invoke
{
  Function f;

  template<class Arg1, class... Args>
  constexpr auto operator()(Arg1&&, Args&&... args) const ->
    decltype(f(std::forward<Args>(args)...))
  {
    return f(std::forward<Args>(args)...);
  }
};


} // end detail


// customization_point is a class for creating Niebler-style customization points
// which attempt one implementation after another, in order, to find an appropriate
// dispatch.
//
// A customization point differs from a multi_function only in that it passes itself
// as the first parameter to each of the potential dispatch functions. Users may use
// detail::drop_first_arg_and_invoke for a simpler interface.
//
// When a customization_point is called like a function:
//
//    (*this)(arg1, args...);
//
// it tries each possible function it was created over, in order:
//
// function1(DerivedOrCP, args...)
// function2(DerivedOrCP, args...)
// ...
//
// The first implementation that is not ill-formed is called.
// If all of these implementations are ill-formed, then the call
// to the customization_point is ill-formed.

template<class Derived, class... Functions>
class customization_point : private multi_function<Functions...>
{
  private:
    using super_t = multi_function<Functions...>;

    using derived_type = std::conditional_t<
      std::is_void<Derived>::value,
      customization_point,
      Derived
    >;

    const derived_type& self() const
    {
      return static_cast<const derived_type&>(*this);
    }

  public:
    constexpr customization_point()
      : customization_point(Functions{}...)
    {}

    constexpr customization_point(Functions... funcs)
      : super_t(funcs...)
    {}

    template<class Arg1, class... Args>
    constexpr auto operator()(Arg1&& arg1, Args&&... args) const ->
      decltype(super_t::operator()(self(), std::forward<Arg1>(arg1), std::forward<Args>(args)...))
    {
      return super_t::operator()(self(), std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    }
};


template<class... Functions, class Derived = void>
constexpr customization_point<Derived,Functions...>
make_customization_point(Functions... funcs)
{
  return customization_point<Derived,Functions...>(funcs...);
}


} // end experimental
