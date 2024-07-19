///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cppcoro/detail/lightweight_manual_reset_event.hpp>
#include <cppcoro/task.hpp>

#include <coroutine>
#include <cassert>
#include <exception>
#include <utility>
#include <iostream>

namespace cppcoro::detail {

class sync_wait_task_promise {
	using coroutine_handle_t = std::coroutine_handle<sync_wait_task_promise>;

public:
	sync_wait_task_promise() = default;

	coroutine_handle_t get_return_object() {
		return coroutine_handle_t::from_promise(*this);
	}

	void start(detail::lightweight_manual_reset_event& event) {
		m_event = &event;
		// Runs make_sync_wait_task
		get_return_object().resume();
	}

	// Do not start executing the task until we call resume().
	std::suspend_always initial_suspend() {
		return{};
	}

	// This is called when the coroutine has completed.
	auto final_suspend() noexcept {
		class completion_notifier {
		public:

			bool await_ready() const noexcept { return false; }

			void await_suspend(coroutine_handle_t coroutine) const noexcept {
				std::cout << "Sync task finished\n";
				// unblocks caller blocked on event.wait()
				coroutine.promise().m_event->set();
			}

			void await_resume() noexcept {}
		};

		return completion_notifier{};
	}

	auto yield_value(std::string& result) {
		m_result = std::addressof(result);
		return final_suspend();
	}

	void return_void() {
		// The coroutine should have either yielded a value or thrown
		// an exception in which case it should have bypassed return_void().
		assert(false);
	}

	void unhandled_exception() {
		m_exception = std::current_exception();
	}

	std::string& result() {
		if (m_exception) {
			std::rethrow_exception(m_exception);
		}
		return static_cast<std::string&>(*m_result);
	}

private:

	detail::lightweight_manual_reset_event* m_event;
	std::string* m_result;
	std::exception_ptr m_exception;

};

class sync_wait_task {
public:

	using promise_type = sync_wait_task_promise;

	using coroutine_handle_t = std::coroutine_handle<promise_type>;

	sync_wait_task(coroutine_handle_t coroutine)
		: m_coroutine(coroutine) {}

	sync_wait_task(sync_wait_task&& other)
		: m_coroutine(std::exchange(other.m_coroutine, coroutine_handle_t{})) {}

	~sync_wait_task() {
		if (m_coroutine) m_coroutine.destroy();
	}

	sync_wait_task(const sync_wait_task&) = delete;
	sync_wait_task& operator=(const sync_wait_task&) = delete;

	void start(lightweight_manual_reset_event& event) {
		m_coroutine.promise().start(event);
	}

	std::string& result() {
		return m_coroutine.promise().result();
	}

private:
	// awaitable
	coroutine_handle_t m_coroutine;
};

sync_wait_task make_sync_wait_task(Task&& awaitable) {
	std::cout << "make_sync_wait_task\n";
	// Evaluates task as if make_sync_wait_task was a coroutine,
	// to "extract" the value
	std::string& value = co_await std::forward<Task>(awaitable);
	// co_await sync_wait_task_promise::yield_value(value); 
	co_yield value;
}
} // namespace cppcoro::detail
