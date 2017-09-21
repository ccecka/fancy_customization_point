#pragma once

#include <utility>

#include "static_const.hpp"
#include "multi_function.hpp"

namespace expdetail
{

struct member_function_invoke
{
  template <class Policy, class... Args>
  constexpr auto operator()(Policy&& policy, Args&&... args) const ->
      decltype(std::forward<Policy>(policy).invoke(std::forward<Args>(args)...))
  {
    return std::forward<Policy>(policy).invoke(std::forward<Args>(args)...);
  }
};

struct free_function_invoke
{
  template <class Policy, class... Args>
  constexpr auto operator()(Policy&& policy, Args&&... args) const ->
      decltype(invoke(std::forward<Policy>(policy), std::forward<Args>(args)...))
  {
    return invoke(std::forward<Policy>(policy), std::forward<Args>(args)...);
  }
};

struct invoke_function
{
  template <class Function, class... Args>
  constexpr auto operator()(Function&& f, Args&&... args) const ->
      decltype(std::forward<Function>(f)(std::forward<Args>(args)...))
  {
    return std::forward<Function>(f)(std::forward<Args>(args)...);
  }
};

} // end namespace expdetail

namespace experimental
{
using invoke_t = detail::multi_function<expdetail::member_function_invoke,
                                        expdetail::free_function_invoke,
                                        expdetail::invoke_function>;
namespace {
constexpr auto const& invoke = detail::static_const<invoke_t>::value;
} // end anon namespace

} // end namespace experimental
