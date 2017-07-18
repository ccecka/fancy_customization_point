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

//
// customization_point is a class for creating Niebler-style customization points
//
// If all of these implementations are ill-formed, then the call to the customization_point is ill-formed.

template<class... Implementations>
class customization_point : multi_function<Implementations...>
{
  private:
    using super_t = multi_function<Implementations...>;

    const super_t& super() const
    {
      return static_cast<const super_t&>(*this);
    }

    template<class Arg1, class... Args>
    static constexpr auto impl(const customization_point& self, Arg1&& arg1, Args&&... args) ->
        decltype(experimental::invoke(std::forward<Arg1>(arg1), self.super(), std::forward<Args>(args)...))
    {
      return experimental::invoke(std::forward<Arg1>(arg1), self.super(), std::forward<Args>(args)...);
    }

    template<class... Args>
    static constexpr auto impl(const super_t& super, Args&&... args) ->
      decltype(super(std::forward<Args>(args)...))
    {
      return super(std::forward<Args>(args)...);
    }

  public:
    constexpr customization_point()
      : customization_point(Implementations{}...)
    {}

    constexpr customization_point(Implementations... funcs)
        : super_t(funcs...)
    {}

    template<class... Args>
    constexpr auto operator()(Args&&... args) const ->
      decltype(customization_point::impl(*this, std::forward<Args>(args)...))
    {
      return customization_point::impl(*this, std::forward<Args>(args)...);
    }
};


template<class... Functions>
constexpr customization_point<Functions...>
  make_customization_point(Functions... funcs)
{
  return customization_point<Functions...>(funcs...);
}


} // end experimental
