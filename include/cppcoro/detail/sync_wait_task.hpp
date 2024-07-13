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

namespace cppcoro::detail {
class sync_wait_task;

class sync_wait_task_promise final
{
	using coroutine_handle_t = std::coroutine_handle<sync_wait_task_promise>;

public:
	sync_wait_task_promise() noexcept
	{}

	void start(detail::lightweight_manual_reset_event& event) {
		m_event = &event;
		coroutine_handle_t::from_promise(*this).resume();
	}

	auto get_return_object() noexcept {
		return coroutine_handle_t::from_promise(*this);
	}

	std::suspend_always initial_suspend() noexcept {
		return{};
	}

	auto final_suspend() noexcept {
		class completion_notifier {
		public:

			bool await_ready() const noexcept { return false; }

			void await_suspend(coroutine_handle_t coroutine) const noexcept
			{
				coroutine.promise().m_event->set();
			}

			void await_resume() noexcept {}
		};

		return completion_notifier{};
	}

	auto yield_value(std::string& result) noexcept {
		m_result = std::addressof(result);
		return final_suspend();
	}

	void return_void() noexcept {
		// The coroutine should have either yielded a value or thrown
		// an exception in which case it should have bypassed return_void().
		assert(false);
	}

	void unhandled_exception() {
		m_exception = std::current_exception();
	}

	std::string& result() {
		if (m_exception)
		{
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

	sync_wait_task(coroutine_handle_t coroutine) noexcept
		: m_coroutine(coroutine) {}

	sync_wait_task(sync_wait_task&& other) noexcept
		: m_coroutine(std::exchange(other.m_coroutine, coroutine_handle_t{})) {}

	~sync_wait_task() {
		if (m_coroutine) m_coroutine.destroy();
	}

	sync_wait_task(const sync_wait_task&) = delete;
	sync_wait_task& operator=(const sync_wait_task&) = delete;

	void start(lightweight_manual_reset_event& event) noexcept {
		m_coroutine.promise().start(event);
	}

	decltype(auto) result() {
		return m_coroutine.promise().result();
	}

private:

	coroutine_handle_t m_coroutine;
};

sync_wait_task make_sync_wait_task(Task&& awaitable) {
	co_yield co_await std::forward<Task>(awaitable);
}
} // namespace cppcoro::detail
