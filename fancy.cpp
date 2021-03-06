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

#include <iostream>
#include <algorithm>

namespace experimental
{

template<class ExecutionPolicy, class... Args>
void default_begin(ExecutionPolicy&&, Args&&... args)
{
  std::cout << "default_begin" << std::endl;
  //static_assert(sizeof(ExecutionPolicy) == 0, "begin: No dispatch found");  // XXX instantiation...
}

} // end namespace experimental


namespace expdetail
{

struct call_member_begin
{
  template<class CP, class Arg1, class... Args>
  constexpr auto operator()(CP&&, Arg1&& arg1, Args&&... args) const ->
      decltype(std::forward<Arg1>(arg1).begin(std::forward<Args>(args)...))
  {
    return std::forward<Arg1>(arg1).begin(std::forward<Args>(args)...);
  }
};

struct call_free_begin
{
  template<class CP, class Arg1, class... Args>
  constexpr auto operator()(CP&& cp, Arg1&& arg1, Args&&... args) const ->
      decltype(experimental::invoke(std::forward<Arg1>(arg1), std::forward<CP>(cp), std::forward<Args>(args)...))
  {
    return experimental::invoke(std::forward<Arg1>(arg1), std::forward<CP>(cp), std::forward<Args>(args)...);
  }

  template<class CP, class... Args>
  constexpr auto operator()(CP&&, Args&&... args) const ->
      decltype(begin(std::forward<Args>(args)...))
  {
    return begin(std::forward<Args>(args)...);
  }
};

struct default_begin
{
  template<class CP, class... Args>
  constexpr auto operator()(CP&&, Args&&... args) const ->
      decltype(experimental::default_begin(std::forward<Args>(args)...))
  {
    return experimental::default_begin(std::forward<Args>(args)...);
  }
};

} // end namespace expdetail


namespace experimental
{

struct begin_t : detail::customization_point<begin_t,
                                             expdetail::call_member_begin,
                                             expdetail::call_free_begin,
                                             expdetail::default_begin>
{};

constexpr begin_t begin{};

} // end namespace experimental


namespace mine
{

struct tag {};

struct me {
  /*
  const int* begin() const {
    std::cout << "member mine begin" << std::endl;
    return &a;
  }
  */
  int a;
};

const int* begin(const me& m) {
  std::cout << "free mine begin" << std::endl;
  return &m.a;
}

const int* begin(tag, const me& m) {
  std::cout << "free tagged mine begin" << std::endl;
  return &m.a;
}

template <class F, class... Args>
auto invoke(tag, F f, Args&&... args)
    -> decltype(f(std::forward<Args>(args)...))
{
  std::cout << "invoke mine" << std::endl;
  return f(std::forward<Args>(args)...);
}

}


namespace yours
{

struct tag {};

template <class F, class... Args>
auto invoke(tag, F f, Args&&... args)
    -> decltype(f(mine::tag{}, std::forward<Args>(args)...))
{
  std::cout << "invoke yours" << std::endl;
  return f(mine::tag{}, std::forward<Args>(args)...);
}

}



namespace other
{

struct tag : yours::tag {};

/*
  const int* begin(tag, const mine::me& t) {
  std::cout << "begin other" << std::endl;
  return my_lib::begin(t);
  }
*/

template <class F, class... Args>
auto invoke(tag, F f, Args&&... args)
    -> decltype(f(yours::tag{}, std::forward<Args>(args)...))
{
  std::cout << "invoke other" << std::endl;
  return f(yours::tag{}, std::forward<Args>(args)...);
}

}



int main()
{
  mine::me m;
  mine::tag  mtag;
  yours::tag ytag;
  other::tag otag;

  using experimental::begin;
  std::cout << begin(m) << std::endl << std::endl;
  std::cout << begin(mtag, m) << std::endl << std::endl;
  std::cout << begin(ytag, m) << std::endl << std::endl;
  std::cout << begin(otag, m) << std::endl << std::endl;
}
