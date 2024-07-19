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
#include <iostream>

namespace cppcoro {

class Task;
class TaskPromise;

struct FinalAwaitable {
	bool await_ready() const noexcept { return false; }

	std::coroutine_handle<> await_suspend(
		std::coroutine_handle<TaskPromise> coro
	) noexcept;

	void await_resume() noexcept {}
};

struct Awaitable {
	std::coroutine_handle<TaskPromise> m_coroutine;

	Awaitable(std::coroutine_handle<TaskPromise> coroutine) noexcept
		: m_coroutine(coroutine) {}

	bool await_ready() const noexcept;

	std::coroutine_handle<> await_suspend(
		std::coroutine_handle<> awaitingCoroutine) noexcept;

	std::string& await_resume();
};

class TaskPromise {
	friend struct FinalAwaitable;	
public:
	static int instanceCount;

	TaskPromise() {
		m_id = instanceCount++;
		print("construct TaskPromise");
	};

	~TaskPromise() {
		if (m_exception) {
			m_exception.~exception_ptr();
		}
	}

	// Construct Task / coroutine_handler from a promise.
	// Part of the promise "interface"
	Task get_return_object() noexcept;

	void unhandled_exception() noexcept {
		::new (static_cast<void*>(std::addressof(m_exception))) std::exception_ptr(
					std::current_exception());
	}

	// Called when we call 
	//
	// co_return value
	//
	// inside the coroutine associated w/ this promise 
	void return_value(const std::string& value) {
		print("Storing value: " + value);
		m_value = value;
	}

	std::string& result() {
		if (m_exception) {
			std::rethrow_exception(m_exception);
		}
		return m_value;
	}

	// Do not start executing the task
	auto initial_suspend() noexcept {
		return std::suspend_always{};
	}

	auto final_suspend() noexcept {
		print("final_suspend");
		return FinalAwaitable{};
	}

	void set_continuation(std::coroutine_handle<> continuation) noexcept {
		m_continuation = continuation;
	}

	void print(const std::string & msg) {
		std::cout << "task[" << m_id << "] " << msg << std::endl;
	}

private:
	std::coroutine_handle<> m_continuation;

	enum class result_type { empty, value, exception };

	result_type m_resultType = result_type::empty;

	std::string m_value;
	std::exception_ptr m_exception;
	int m_id = -1;
};

int TaskPromise::instanceCount = 0;
 
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

private:


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

	~Task() = default;

	// Example call:
	//
	// std::string r = co_await some_coroutine();
	//
	// This corresponds to:
	//
	// Task coro = some_coroutine();
	// Awaitable a = coro.co_await();
	Awaitable operator co_await() const & noexcept {
		m_coroutine.promise().print("co_await");
		return Awaitable{ m_coroutine };
	}

private:
	std::coroutine_handle<promise_type> m_coroutine;
};

std::coroutine_handle<> FinalAwaitable::await_suspend(
	std::coroutine_handle<TaskPromise> coro
) noexcept {
	return coro.promise().m_continuation;
}

std::coroutine_handle<> Awaitable::await_suspend(
	std::coroutine_handle<> awaitingCoroutine
) noexcept {
	m_coroutine.promise().print("[Awaitable] await_suspend");
	m_coroutine.promise().set_continuation(awaitingCoroutine);
	return m_coroutine;
}

bool Awaitable::await_ready() const noexcept {
	bool is_ready = !m_coroutine || m_coroutine.done();
	std::string msg = "[Awaitable] await_ready? ";
	this->m_coroutine.promise().print(
		msg + (is_ready ? "yes": "no")
	);
	return is_ready;
}

std::string& Awaitable::await_resume() {
	if (!this->m_coroutine) {
		throw std::runtime_error("broken promise");
	}
	this->m_coroutine.promise().print("[Awaitable] await_resume");

	return this->m_coroutine.promise().result();
}

Task TaskPromise::get_return_object() noexcept {
	print("constructing Task");
	return Task{ std::coroutine_handle<TaskPromise>::from_promise(*this) };
}
} // namespace cppcoro
