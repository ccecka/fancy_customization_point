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
    return std::forward<Customizer>(customizer).invoke(std::forward<Args>(args)...);
  }
};


struct free_function_invoke_with_customizer
{
  template<class Customizer, class... Args>
  constexpr auto operator()(Customizer&& customizer, Args&&... args) const ->
    decltype(invoke(std::forward<Customizer>(customizer), std::forward<Args>(args)...))
  {
    return invoke(std::forward<Customizer>(customizer), std::forward<Args>(args)...);
  }
};


struct invoke_function_directly
{
  template<class Function, class... Args>
  constexpr auto operator()(Function&& f, Args&&... args) const ->
    decltype(std::forward<Function>(f)(std::forward<Args>(args)...))
  {
    return std::forward<Function>(f)(std::forward<Args>(args)...);
  }
};

} // end detail

// invoke(arg1, args...) has four cases implemented by the functors above:
//
//   1. Assume arg1 is the customizer. Try calling arg1.invoke(args...)
//   2. Assume arg1 is the customizer. Try calling invoke(arg1, args...) via ADL
//   3. Assume arg1 is a function. Try calling arg1(args...) like a function
//   4. Drop the first argument (presumably a customizer type which didn't happen to provide a customization) and recurse to experimental::invoke(args...)

class invoke_t : private multi_function<
  detail::member_function_invoke_with_customizer,
  detail::free_function_invoke_with_customizer,
  detail::invoke_function_directly
>
{
  private:
    using super_t = multi_function<
      detail::member_function_invoke_with_customizer,
      detail::free_function_invoke_with_customizer,
      detail::invoke_function_directly
    >;

  public:
    using super_t::super_t;

    template<class... Args>
    constexpr auto operator()(Args&&... args) const ->
      decltype(super_t::operator()(std::forward<Args>(args)...))
    {
      // when this invoke_t is called like a function, it inserts itself as the first parameter to the call
      // to the multi_function
      // this allows the recursion used in drop_customizer_and_invoke_with_self above
      return super_t::operator()(std::forward<Args>(args)...);
    }
};


constexpr invoke_t invoke{};


} // end experimental
