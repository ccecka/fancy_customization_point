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

#pragma once

#include "customization_point.hpp"
#include "invoke.hpp"
#include <algorithm>

namespace experimental
{

template<class Iterator, class Function>
Function default_for_each(Iterator first, Iterator last, Function f)
{
  return std::for_each(first, last, f);
}

template<class ExecutionPolicy, class Iterator, class Function>
void default_for_each(ExecutionPolicy&&, Iterator first, Iterator last, Function f)
{
  std::for_each(first, last, f);
}

} // end namespace experimental

namespace expdetail
{

struct call_member_for_each
{
  template<class CP, class Arg1, class... Args>
  constexpr auto operator()(CP&&, Arg1&& arg1, Args&&... args) const ->
    decltype(std::forward<Arg1>(arg1).for_each(std::forward<Args>(args)...))
  {
    return std::forward<Arg1>(arg1).for_each(std::forward<Args>(args)...);
  }
};

struct call_free_for_each
{
  template<class CP, class Arg1, class... Args>
  constexpr auto operator()(CP&& cp, Arg1&& arg1, Args&&... args) const ->
    decltype(invoke(std::forward<Arg1>(arg1), std::forward<CP>(cp), std::forward<Args>(args)...))
  {
    return invoke(std::forward<Arg1>(arg1), std::forward<CP>(cp), std::forward<Args>(args)...);
  }

  template<class CP, class... Args>
  constexpr auto operator()(CP&&, Args&&... args) const ->
    decltype(for_each(std::forward<Args>(args)...))
  {
    return for_each(std::forward<Args>(args)...);
  }
};

struct default_for_each
{
  template<class CP, class... Args>
  constexpr auto operator()(CP&&, Args&&... args) const ->
    decltype(experimental::default_for_each(std::forward<Args>(args)...))
  {
    return experimental::default_for_each(std::forward<Args>(args)...);
  }
};

} // end namespace expdetail

namespace experimental
{

struct for_each_t : detail::customization_point<for_each_t,
                                                expdetail::call_member_for_each,
                                                expdetail::call_free_for_each,
                                                expdetail::default_for_each>
{};

namespace {
constexpr auto const& for_each = detail::static_const<for_each_t>::value;
} // end anon namespace

} // end namespace experimental

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
