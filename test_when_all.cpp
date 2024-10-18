// compile.sh

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/resume_on.hpp>
#include <cppcoro/io_service.hpp>

#include <iostream>
#include <thread>
#include <chrono>

namespace cppcoro {

Task identity(std::string input = "default") {
    co_return input;
}

Task f() {

    std::vector<Task> tasks;
    tasks.emplace_back(identity("hello"));
    tasks.emplace_back(identity("world"));
    // when_all_ready returns a WhenAllReadyAwaitable
    // WhenAllReadyAwaitable.co_await() returns a InternalAwaiter
    // calls InternalAwaiter::await_suspend(f)
    // std::vector<WhenAllTask> result = InternalAwaiter::await_resume()
    std::vector<detail::WhenAllTask> awaited_tasks = 
        co_await when_all_ready(std::move(tasks));
    std::string r1 = awaited_tasks[0].result();
    std::string r2 = awaited_tasks[1].result();
    co_return r1 + r2;
}

Task g() {
    io_service io;
    co_await schedule_on(io, f());
    co_return "";
}

} // namespace cppcoro

int main() {
  std::string r = cppcoro::sync_wait(cppcoro::g());
  std::cout << r << std::endl;
  return 0;
}
