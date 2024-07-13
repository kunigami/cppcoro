#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <iostream>

namespace cppcoro {


task<std::string> identity(std::string input) {
  co_return input;
}

task<std::string> hw() {
  auto a = co_await identity("hello");
  auto b = co_await identity("world");
  co_return a + b; 
}

task<std::string> hw2() {
  auto a = co_await hw();
  auto b = co_await hw();
  co_return a + b; 
}

task<std::string> run() {
  auto a = co_await hw2();
  std::cout << a << std::endl;
  co_return "";
}

}

int main() {
  cppcoro::sync_wait(cppcoro::run());
  #if CPPCORO_COMPILER_SUPPORTS_SYMMETRIC_TRANSFER
  std::cout << "CPPCORO_COMPILER_SUPPORTS_SYMMETRIC_TRANSFER" << std::endl;
  #endif
  return 0;
}
