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

#include <iostream>
#include <typeinfo>
#include <vector>
#include <algorithm>
#include <numeric>

#include "for_each.hpp"


namespace mine
{


struct seq_policy
{
  // customize for_each by introducing a member function
  template<class Iterator, class Function>
  void for_each(Iterator first, Iterator last, Function f) const
  {
    std::cout << "mine::seq_policy::for_each()" << std::endl;
    experimental::for_each(first, last, f);
  }
};

constexpr seq_policy seq{};


struct fancy_policy {};
constexpr fancy_policy fancy{};


template<class FancyExecutionPolicy,
         class CustomizationPoint, class... Args,
         class = std::enable_if_t<
           std::is_same<fancy_policy, std::decay_t<FancyExecutionPolicy>>::value
         >>
auto invoke(FancyExecutionPolicy&& policy, CustomizationPoint&& customization_point, Args&&... args)
{
  std::cout << "mine::invoke(fancy, " << typeid(CustomizationPoint).name() << ", args...)" << std::endl;

  // call the customization point with seq
  return customization_point(seq, std::forward<Args>(args)...);

  // Because customization_points are callable objects, they can be invoked via experimental::invoke()
  // Therefore, the above call ends up being equivalent to:
  //
  //     return experimental::invoke(customization_point, seq, std::forward<Args>(args)...);
  //
  // Moreover, because there is no specialization of invoke() for seq, the above call is also equivalent to:
  //
  //     return experimental::invoke(seq, customization_point, std::forward<Args>(args)...);
  //
  // This is because experimental::invoke will not find a specialization of invoke(seq, customization_point, args...).
  // Therefore, experimental::invoke will attempt the following call, which will succeed:
  //
  //     return customization_point(std::forward<Args>(args)...);
}


}

int main()
{
  std::vector<int> vec(10);
  std::iota(vec.begin(), vec.end(), 0);

  // should just print the numbers
  experimental::for_each(vec.begin(), vec.end(), [](int x)
  {
    std::cout << x << " ";
  });

  std::cout << std::endl;
  std::cout << std::endl;

  // should print something like mine::seq_policy::for_each(),
  // and then the numbers
  experimental::for_each(mine::seq, vec.begin(), vec.end(), [](int x)
  {
    std::cout << x << " ";
  });

  std::cout << std::endl;
  std::cout << std::endl;

  // should print something fancy like mine::invoke(fancy, for_each_t),
  // and then something like mine::seq_policy::for_each(),
  // and then the numbers
  experimental::for_each(mine::fancy, vec.begin(), vec.end(), [](int x)
  {
    std::cout << x << " ";
  });

  std::cout << std::endl;
  std::cout << std::endl;

  std::cout << "OK" << std::endl;
}
