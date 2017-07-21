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


namespace detail
{

struct call_member_begin
{
  template<class Arg1, class... Args>
  constexpr auto operator()(Arg1&& arg1, Args&&... args) const ->
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
  template<class... Args>
  constexpr auto operator()(Args&&... args) const ->
      decltype(experimental::default_begin(std::forward<Args>(args)...))
  {
    return experimental::default_begin(std::forward<Args>(args)...);
  }
};


} // end detail


struct begin_t : experimental::customization_point<begin_t,
                                                   detail::drop_first_arg_and_invoke<detail::call_member_begin>,
                                                   detail::call_free_begin,
                                                   detail::drop_first_arg_and_invoke<detail::default_begin>>
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
