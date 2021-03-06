/******************************************************************************
 * Copyright (C) 2016-2017, Cris Cecka.  All rights reserved.
 * Copyright (C) 2016-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include "customization_point.hpp"
#include "invoke.hpp"

#include "print_type.hpp"

#include <iostream>
#include <algorithm>

namespace experimental
{

template<class ExecutionPolicy, class... Args>
void default_dot(ExecutionPolicy&&, Args&&... args)
{
  std::cout << "default_dot" << std::endl;
  //static_assert(sizeof(ExecutionPolicy) == 0, "dot: No dispatch found");
}

} // end namespace experimental


namespace expdetail
{

struct call_member_dot
{
  template<class CP, class Arg1, class... Args>
  constexpr auto operator()(CP&&, Arg1&& arg1, Args&&... args) const ->
    decltype(std::forward<Arg1>(arg1).dot(std::forward<Args>(args)...))
  {
    return std::forward<Arg1>(arg1).dot(std::forward<Args>(args)...);
  }
};

struct call_free_dot
{
  template<class CP, class Arg1, class... Args>
  constexpr auto operator()(CP&& cp, Arg1&& arg1, Args&&... args) const ->
      decltype(experimental::invoke(std::forward<Arg1>(arg1), std::forward<CP>(cp), std::forward<Args>(args)...))
  {
    return experimental::invoke(std::forward<Arg1>(arg1), std::forward<CP>(cp), std::forward<Args>(args)...);
  }

  template<class CP, class... Args>
  constexpr auto operator()(CP&&, Args&&... args) const ->
    decltype(dot(std::forward<Args>(args)...))
  {
    return dot(std::forward<Args>(args)...);
  }
};

struct default_dot
{
  template<class CP, class... Args>
  constexpr auto operator()(CP&&, Args&&... args) const ->
    decltype(experimental::default_dot(std::forward<Args>(args)...))
  {
    return experimental::default_dot(std::forward<Args>(args)...);
  }
};

} // end namespace expdetail


namespace experimental
{

struct dot_t : detail::customization_point<dot_t,
                                           expdetail::call_member_dot,
                                           expdetail::call_free_dot,
                                           expdetail::default_dot>
{};

namespace {
constexpr auto const& dot = detail::static_const<dot_t>::value;
} // end anon namespace

} // end namespace experimental



namespace mine {
struct tag {};

void dot(tag) { std::cout << "dot mine::tag" << std::endl; }
}


namespace yours {
struct tag {};


template <class F>
void invoke(tag, F f) {
  std::cout << "invoke yours::tag" << std::endl;
  return f(mine::tag{});
}

}



#include "disable_function.hpp"

namespace other {

template <class DerivedPolicy>
struct execution_policy : DerivedPolicy {
  mutable std::string prefix_;
};


template <class Function, class DerivedPolicy, class... T>
void
invoke(const execution_policy<DerivedPolicy>& exec, Function f, T&&... t)
{
  std::cout << exec.prefix_ << type_name<Function>() << "(" << type_name<DerivedPolicy>();
  if (sizeof...(T) > 0) std::cout << ", ";
  print_all(std::forward<T>(t)...);
  std::cout << ")" << std::endl;
  exec.prefix_ += "  ";

  f(experimental::prefer_customization_point<DerivedPolicy, Function>(exec), std::forward<T>(t)...);

  exec.prefix_.erase(exec.prefix_.size()-2);
}

} // end namespace other



int main()
{
  using namespace experimental;

  dot(mine::tag{});
  std::cout << std::endl;

  dot(yours::tag{});
  std::cout << std::endl;

  {
  other::execution_policy<mine::tag> exec;
  dot(exec);
  }
  std::cout << std::endl;

  {
  other::execution_policy<yours::tag> exec;
  dot(exec);
  }
  std::cout << std::endl;

}
