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

// cppcoro::task<> usage_example()
// {
//   // Calling function creates a new task but doesn't start
//   // executing the coroutine yet.
//   cppcoro::task<int> countTask = count_lines("foo.txt");

//   // ...

//   // Coroutine is only started when we later co_await the task.
//   int lineCount = co_await countTask;

//   std::cout << "line count = " << lineCount << std::endl;
// }

task<> run() {
  auto a = co_await hw();
  std::cout << a << std::endl;
  co_return;
}

}

int main() {
  cppcoro::sync_wait(cppcoro::run());
  
  return 0;
}
