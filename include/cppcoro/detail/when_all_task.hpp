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

class WhenAllTaskPromise;

class CompletionNotifier {
public:

	bool await_ready() const noexcept { return false; }

	void await_suspend(std::coroutine_handle<WhenAllTaskPromise> coro) const noexcept;

	void await_resume() const noexcept {}
};


// This promise is associated with the function make_when_all_task()
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

	CompletionNotifier final_suspend() noexcept {
		assert(false);
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

	// Saves the value internally, the caller will retrieve this
	// using .result().
	CompletionNotifier yield_value(std::string& result) noexcept {
		m_result = std::addressof(result);
		return CompletionNotifier{};
	}

	// This promise starts suspended. Resume execution when 
	// start() is called explicitly by WhenAllReadyAwaitable
	void start(WhenAllCounter& counter) noexcept {
		std::cout << "[WhenAllTaskPromise] start" << std::endl;
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

	friend class CompletionNotifier;

	void rethrow_if_exception() {
		if (m_exception) {
			std::rethrow_exception(m_exception);
		}
	}

	WhenAllCounter* m_counter;
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

	void start(WhenAllCounter& counter) noexcept {
		m_coroutine.promise().start(counter);
	}

	coroutine_handle_t m_coroutine;

};


void CompletionNotifier::await_suspend(std::coroutine_handle<WhenAllTaskPromise> coro) const noexcept {
	std::cout << "[CompletionNotifier] await_suspend()" << std::endl;
	coro.promise().m_counter->notify_awaitable_completed();
}

WhenAllTask make_when_all_task(Task&& awaitable) {
	std::cout << "[make_when_all_task] begin" << std::endl;
	// Calls Task::co_await() -> Awaitable
	// Awaitable::await_suspend()
	// ...
	// Gets resumed by FinalAwaitable::await_resume()
	//
	// std::string value = Awaitable::await_resume()
	std::string value = co_await awaitable;
	std::cout << "[make_when_all_task] got value: " << value << std::endl;
	// awaitable = WhenAllTaskPromise::yield_value()
	// - stores `value` internally
	// - returns final_suspend(): CompletionNotifier
	// co_await awaitable
	//	- CompletionNotifier::await_suspend()
	//     - WhenAllTaskPromise::m_counter->notify_awaitable_completed
	co_yield value;
}

} // namespace cppcoro::detail
