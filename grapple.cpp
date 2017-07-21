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


namespace detail
{

struct call_member_dot
{
  template<class Arg1, class... Args>
  constexpr auto operator()(Arg1&& arg1, Args&&... args) const ->
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
  template<class... Args>
  constexpr auto operator()(Args&&... args) const ->
    decltype(experimental::default_dot(std::forward<Args>(args)...))
  {
    return experimental::default_dot(std::forward<Args>(args)...);
  }
};


} // end detail


struct dot_t : experimental::customization_point<dot_t,
                                                 detail::drop_first_arg_and_invoke<detail::call_member_dot>,
                                                 detail::call_free_dot,
                                                 detail::drop_first_arg_and_invoke<detail::default_dot>>
{};

constexpr dot_t dot{};

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

  using namespace disable;
  f(remove_customization_point<Function>(exec), std::forward<T>(t)...);

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

  /*
    // XXX TODO
    // invoke(remove_customization_point, ...) = delete
    // gets applied to all base classes of remove_customization_point as well so that
    // invoke(yours::tag, ...) is not being found.
  {
  other::execution_policy<yours::tag> exec;
  dot(exec);
  }
  std::cout << std::endl;
  */
}
