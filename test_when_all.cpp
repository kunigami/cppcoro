// g++ -std=c++20 test_when_all.cpp -Iinclude/ lib/lightweight_manual_reset_event.cpp

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>

namespace cppcoro {
Task identity(std::string input = "default") {
  co_return input;
}

Task f() {

    std::vector<Task> tasks;
    tasks.emplace_back(identity("hello"));
    tasks.emplace_back(identity("world"));
    auto awaited_tasks = co_await when_all_ready(std::move(tasks));
    std::string r1 = awaited_tasks[0].result();
    std::string r2 = awaited_tasks[1].result();
    co_return r1 + r2;
}
} // namespace cppcoro

int main() {
  std::string r = cppcoro::sync_wait(cppcoro::f());
  std::cout << r << std::endl;
  return 0;
}
