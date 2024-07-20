///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cppcoro/detail/when_all_counter.hpp>

#include <cppcoro/detail/when_all_task.hpp>

#include <coroutine>
#include <tuple>
#include <vector>

namespace cppcoro::detail {

class WhenAllReadyAwaitable {
public:

	explicit WhenAllReadyAwaitable(std::vector<WhenAllTask>&& tasks) noexcept
		: m_counter(tasks.size())
		, m_tasks(std::move(tasks)) {}

	WhenAllReadyAwaitable(const WhenAllReadyAwaitable&) = delete;
	WhenAllReadyAwaitable& operator=(const WhenAllReadyAwaitable&) = delete;

	auto operator co_await() & noexcept {
		class InternalAwaiter {
		public:

			InternalAwaiter(WhenAllReadyAwaitable& awaitable)
				: m_awaitable(awaitable) {}

			bool await_ready() const noexcept {
				return m_awaitable.is_ready();
			}

			bool await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept {
				return m_awaitable.try_await(awaitingCoroutine);
			}

			std::vector<WhenAllTask>& await_resume() noexcept {
				return m_awaitable.m_tasks;
			}

		private:

			WhenAllReadyAwaitable& m_awaitable;
		};

		return InternalAwaiter{ *this };
	}


	auto operator co_await() && noexcept {
		class InternalAwaiter {
		public:

			InternalAwaiter(WhenAllReadyAwaitable& awaitable)
				: m_awaitable(awaitable) {}

			bool await_ready() const noexcept {
				return m_awaitable.is_ready();
			}

			bool await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept {
				return m_awaitable.try_await(awaitingCoroutine);
			}

			std::vector<WhenAllTask>&& await_resume() noexcept {
				return std::move(m_awaitable.m_tasks);
			}

		private:

			WhenAllReadyAwaitable& m_awaitable;
		};

		return InternalAwaiter{ *this };
	}

private:

	bool is_ready() const noexcept {
		return m_counter.is_ready();
	}

	bool try_await(std::coroutine_handle<> awaitingCoroutine) noexcept {
		std::cout << "[WhenAllReadyAwaitable] try_await" << std::endl;
		for (auto&& task : m_tasks) {
			task.start(m_counter);
		}

		return m_counter.try_await(awaitingCoroutine);
	}

	WhenAllCounter m_counter;
	std::vector<WhenAllTask> m_tasks;
};
} // namespace cppcoro::detail
