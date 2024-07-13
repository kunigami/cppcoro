///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <atomic>
#include <exception>
#include <utility>
#include <cstdint>
#include <cassert>

#include <coroutine>

namespace cppcoro {

class Task;

struct final_awaitable {
	bool await_ready() const noexcept { return false; }

	template<typename PROMISE>
	std::coroutine_handle<> await_suspend(
		std::coroutine_handle<PROMISE> coro) noexcept
	{
		return coro.promise().m_continuation;
	}

	void await_resume() noexcept {}
};

class TaskPromise {
	friend struct final_awaitable;
public:

	TaskPromise() noexcept {}

	~TaskPromise() {
		switch (m_resultType)
		{
		case result_type::value:
			break;
		case result_type::exception:
			m_exception.~exception_ptr();
			break;
		default:
			break;
		}
	}

	Task get_return_object() noexcept;

	void unhandled_exception() noexcept {
	}

	template<
		typename VALUE,
		typename = std::enable_if_t<std::is_convertible_v<VALUE&&, std::string>>>
	void return_value(VALUE&& value) {
		::new (static_cast<void*>(std::addressof(m_value))) std::string(std::forward<VALUE>(value));
		m_resultType = result_type::value;
	}

	std::string& result() &
	{
		if (m_resultType == result_type::exception)
		{
			std::rethrow_exception(m_exception);
		}

		assert(m_resultType == result_type::value);

		return m_value;
	}

	auto initial_suspend() noexcept {
		return std::suspend_always{};
	}

	auto final_suspend() noexcept {
		return final_awaitable{};
	}

	void set_continuation(std::coroutine_handle<> continuation) noexcept {
		m_continuation = continuation;
	}

private:
	std::coroutine_handle<> m_continuation;

	enum class result_type { empty, value, exception };

	result_type m_resultType = result_type::empty;

	union
	{
		std::string m_value;
		std::exception_ptr m_exception;
	};

};
 
/// \brief
/// A Task represents an operation that produces a result both lazily
/// and asynchronously.
///
/// When you call a coroutine that returns a Task, the coroutine
/// simply captures any passed parameters and returns exeuction to the
/// caller. Execution of the coroutine body does not start until the
/// coroutine is first co_await'ed.
class Task {
public:

	using promise_type = TaskPromise;

	using value_type = std::string;

private:

	struct awaitable_base {
		std::coroutine_handle<promise_type> m_coroutine;

		awaitable_base(std::coroutine_handle<promise_type> coroutine) noexcept
			: m_coroutine(coroutine)
		{}

		bool await_ready() const noexcept {
			return !m_coroutine || m_coroutine.done();
		}

		std::coroutine_handle<> await_suspend(
			std::coroutine_handle<> awaitingCoroutine) noexcept {
			m_coroutine.promise().set_continuation(awaitingCoroutine);
			return m_coroutine;
		}
	};

public:

	Task() noexcept
		: m_coroutine(nullptr) {}

	explicit Task(std::coroutine_handle<promise_type> coroutine)
		: m_coroutine(coroutine) {}

	Task(Task&& t) noexcept
		: m_coroutine(t.m_coroutine) {
		t.m_coroutine = nullptr;
	}

	Task(const Task&) = delete;
	Task& operator=(const Task&) = delete;

	~Task() {
		if (m_coroutine) {
			m_coroutine.destroy();
		}
	}

	Task& operator=(Task&& other) noexcept {
		if (std::addressof(other) != this) {
			if (m_coroutine)
			{
				m_coroutine.destroy();
			}

			m_coroutine = other.m_coroutine;
			other.m_coroutine = nullptr;
		}

		return *this;
	}

	/// \brief
	/// Query if the Task result is complete.
	///
	/// Awaiting a Task that is ready is guaranteed not to block/suspend.
	bool is_ready() const noexcept {
		return !m_coroutine || m_coroutine.done();
	}

	auto operator co_await() const & noexcept {
		struct awaitable : awaitable_base {
			using awaitable_base::awaitable_base;

			decltype(auto) await_resume()
			{
				if (!this->m_coroutine) {
					throw std::runtime_error("broken promise");
				}

				return this->m_coroutine.promise().result();
			}
		};

		return awaitable{ m_coroutine };
	}

	/// \brief
	/// Returns an awaitable that will await completion of the Task without
	/// attempting to retrieve the result.
	auto when_ready() const noexcept {
		struct awaitable : awaitable_base {
			using awaitable_base::awaitable_base;

			void await_resume() const noexcept {}
		};
		return awaitable{ m_coroutine };
	}

private:
	std::coroutine_handle<promise_type> m_coroutine;
};

Task TaskPromise::get_return_object() noexcept {
	return Task{ std::coroutine_handle<TaskPromise>::from_promise(*this) };
}
} // namespace cppcoro
