#pragma once

#include "customization_point.hpp"
#include "invoke.hpp"
#include <algorithm>

namespace experimental
{


template<class Iterator, class Function>
Function default_for_each(Iterator first, Iterator last, Function f)
{
  std::cout << "Default for each" << std::endl;
  return std::for_each(first, last, f);
}

template<class ExecutionPolicy, class Iterator, class Function>
void default_for_each(ExecutionPolicy&&, Iterator first, Iterator last, Function f)
{
  std::cout << "Default for each w policy" << std::endl;
  std::for_each(first, last, f);
}


namespace detail
{

struct call_member_for_each
{
  template<class Arg1, class... Args>
  constexpr auto operator()(Arg1&& arg1, Args&&... args) const ->
    decltype(std::forward<Arg1>(arg1).for_each(std::forward<Args>(args)...))
  {
    std::cout << "Member for each" << std::endl;
    return std::forward<Arg1>(arg1).for_each(std::forward<Args>(args)...);
  }
};

struct call_free_for_each
{
  template<class... Args>
  constexpr auto operator()(Args&&... args) const ->
    decltype(for_each(std::forward<Args>(args)...))
  {
    std::cout << "Free for each" << std::endl;
    return for_each(std::forward<Args>(args)...);
  }
};

struct default_for_each
{
  template<class... Args>
  constexpr auto operator()(Args&&... args) const ->
    decltype(experimental::default_for_each(std::forward<Args>(args)...))
  {
    return experimental::default_for_each(std::forward<Args>(args)...);
  }
};


} // end detail


constexpr auto for_each = experimental::make_customization_point(
  detail::call_member_for_each{},
  detail::call_free_for_each{},
  detail::default_for_each{}
);

// or, in C++17 with constexpr lambda:
//
//     constexpr auto for_each = experimental::make_customization_point(
//       [](auto&& policy, auto... args)
//       {
//         return policy.for_each(args...);
//       },
//       [](auto&& policy, auto... args)
//       {
//         return for_each(policy, args...);
//       },
//       [](auto&& policy, auto... args)
//       {
//         return experimental::default_for_each(policy, args...);
//       }
//     );


// the following way to define for_each by deriving from customization_point gives its type a unique, friendly name "for_each_t":
//
//     struct for_each_t : experimental::customization_point<for_each_t, detail::call_member_for_each, detail::call_free_for_each, detail::default_for_each> {};
//
//     constexpr for_each_t for_each{};


} // end experimental
