///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <coroutine>
#include <atomic>
#include <cstdint>
#include <optional>

#include <cppcoro/task.hpp>

namespace cppcoro::detail {
class WhenAllCounter {
public:

	WhenAllCounter(std::size_t count) noexcept
		: m_count(count),
		m_awaitingCoroutine(nullptr)
	{}

	bool is_ready() const noexcept {
		// We consider this complete if we're asking whether it's ready
		// after a coroutine has already been registered.
		return m_awaitingCoroutine != nullptr;
	}

	// Stores a handle to the caller coroutine so we can resume it when 
	// all tasks have completed.
	//
	// If all the other tasks have already been completed, we'll return true
	// this will cause the caller to resume immediately.
	bool try_await(std::coroutine_handle<TaskPromise> awaitingCoroutine) noexcept {
		m_awaitingCoroutine = awaitingCoroutine;
		return m_count.fetch_sub(1, std::memory_order_acq_rel) > 0;
	}

	// When a task completes, it calls this method
	// If all task complete, it resumes the original caller of
	// "when_all_ready()"
	void notify_awaitable_completed() noexcept {
		if (m_count.fetch_sub(1, std::memory_order_acq_rel) == 0) {
			m_awaitingCoroutine.resume();
		}
	}

protected:

	// Number of tasks being awaited.
	std::atomic<std::size_t> m_count;
	// Handle to the function that called "when_all_ready()"
	std::coroutine_handle<TaskPromise> m_awaitingCoroutine;
};

} // namespace cppcoro::detail
