#pragma once

#include "multi_function.hpp"
#include <utility>


namespace experimental
{
namespace detail
{


struct member_function_invoke_with_customizer
{
  template<class Customizer, class... Args>
  constexpr auto operator()(Customizer&& customizer, Args&&... args) const ->
    decltype(std::forward<Customizer>(customizer).invoke(std::forward<Args>(args)...))
  {
    std::cout << "customizer.invoke(args...)" << std::endl;
    return std::forward<Customizer>(customizer).invoke(std::forward<Args>(args)...);
  }
};


struct free_function_invoke_with_customizer
{
  template<class Customizer, class... Args>
  constexpr auto operator()(Customizer&& customizer, Args&&... args) const ->
    decltype(invoke(std::forward<Customizer>(customizer), std::forward<Args>(args)...))
  {
    std::cout << "Free invoke" << std::endl;
    return invoke(std::forward<Customizer>(customizer), std::forward<Args>(args)...);
  }
};


struct invoke_function_directly
{
  template<class Function, class... Args>
  constexpr auto operator()(Function&& f, Args&&... args) const ->
    decltype(std::forward<Function>(f)(std::forward<Args>(args)...))
  {
    std::cout << "invoke -> f(args...)" << std::endl;
    return std::forward<Function>(f)(std::forward<Args>(args)...);
  }
};

} // end detail

// invoke(arg1, args...) has four cases implemented by the functors above:
//
//   1. Assume arg1 is the customizer. Try calling arg1.invoke(args...)
//   2. Assume arg1 is the customizer. Try calling invoke(arg1, args...) via ADL
//   3. Assume arg1 is a function. Try calling arg1(args...) like a function

struct invoke_t : multi_function<
  detail::member_function_invoke_with_customizer,
  detail::free_function_invoke_with_customizer,
  detail::invoke_function_directly
  >
{};

constexpr invoke_t invoke{};


} // end experimental
