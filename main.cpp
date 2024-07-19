#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <iostream>

namespace cppcoro {


Task identity(std::string input = "default") {
  co_return input;
}

Task hw() {
  std::cout << "[hw] start" << std::endl;
  // awaitable_a = identity("hello").co_await()
  // awaitable_a.await_suspend()
  // <execute identity()>
  // a = awaitable_a.resume()
  auto a = co_await identity("hello");
  std::cout << "[hw] after a" << std::endl;
  // awaitable_b = identity("world").co_await()
  // awaitable_b.await_suspend()
  // <execute identity()>
  // b = awaitable_b.resume()
  auto b = co_await identity("world");
  std::cout << "[hw] after b" << std::endl;
  co_return a + b; 
}

Task hw2() {
  std::cout << "[h2] start" << std::endl;
  auto a = co_await hw();
  std::cout << "[h2] after a" << std::endl;
  auto b = co_await hw();
  std::cout << "[h2] after b" << std::endl;
  co_return a + b; 
}

Task f() {
  std::cout << "running f" << std::endl;
  auto a = co_await hw();
  std::cout << a << std::endl;
  co_return "";
}

}

int main() {
  cppcoro::sync_wait(cppcoro::hw());
  #if CPPCORO_COMPILER_SUPPORTS_SYMMETRIC_TRANSFER
  std::cout << "CPPCORO_COMPILER_SUPPORTS_SYMMETRIC_TRANSFER" << std::endl;
  #endif
  return 0;
}
