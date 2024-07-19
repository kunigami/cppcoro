///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cppcoro/awaitable_traits.hpp>

#include <cppcoro/detail/when_all_counter.hpp>
#include <cppcoro/detail/void_value.hpp>

#include <coroutine>
#include <cassert>

namespace cppcoro::detail {

class WhenAllReadyAwaitable;

class WhenAllTask;

class WhenAllTaskPromise final {
public:

	using coroutine_handle_t = std::coroutine_handle<WhenAllTaskPromise>;

	WhenAllTaskPromise() noexcept {}

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

			void await_suspend(coroutine_handle_t coro) const noexcept {
				coro.promise().m_counter->notify_awaitable_completed();
			}

			void await_resume() const noexcept {}
		};

		return completion_notifier{};
	}

	void unhandled_exception() noexcept {
		m_exception = std::current_exception();
	}

	void return_void() noexcept {
		// We should have either suspended at co_yield point or
		// an exception was thrown before running off the end of
		// the coroutine.
		assert(false);
	}

	auto yield_value(std::string& result) noexcept {
		m_result = std::addressof(result);
		return final_suspend();
	}

	void start(when_all_counter& counter) noexcept {
		m_counter = &counter;
		coroutine_handle_t::from_promise(*this).resume();
	}

	std::string& result() & {
		rethrow_if_exception();
		return *m_result;
	}

	std::string&& result() && {
		rethrow_if_exception();
		return std::forward<std::string&&>(*m_result);
	}

private:

	void rethrow_if_exception() {
		if (m_exception) {
			std::rethrow_exception(m_exception);
		}
	}

	when_all_counter* m_counter;
	std::exception_ptr m_exception;
	std::add_pointer_t<std::string&> m_result;

};

class WhenAllTask final {
public:

	using promise_type = WhenAllTaskPromise;

	using coroutine_handle_t = typename promise_type::coroutine_handle_t;

	WhenAllTask(coroutine_handle_t coroutine) noexcept
		: m_coroutine(coroutine) {}

	WhenAllTask(WhenAllTask&& other) noexcept
		: m_coroutine(std::exchange(other.m_coroutine, coroutine_handle_t{})) {}

	~WhenAllTask() {
		if (m_coroutine) m_coroutine.destroy();
	}

	WhenAllTask(const WhenAllTask&) = delete;
	WhenAllTask& operator=(const WhenAllTask&) = delete;

	decltype(auto) result() & {
		return m_coroutine.promise().result();
	}

	decltype(auto) result() && {
		return std::move(m_coroutine.promise()).result();
	}

	decltype(auto) non_void_result() & {
		return this->result();
	}

	decltype(auto) non_void_result() && {
		return std::move(*this).result();
	}

private:

	friend class WhenAllReadyAwaitable;

	void start(when_all_counter& counter) noexcept {
		m_coroutine.promise().start(counter);
	}

	coroutine_handle_t m_coroutine;

};

WhenAllTask make_when_all_task(Task&& awaitable) {
	co_yield co_await static_cast<Task&&>(awaitable);
}

} // namespace cppcoro::detail
