///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cppcoro/config.hpp>
#include <cppcoro/awaitable_traits.hpp>
#include <cppcoro/is_awaitable.hpp>
#include <cppcoro/task.hpp>

#include <cppcoro/detail/when_all_ready_awaitable.hpp>
#include <cppcoro/detail/when_all_task.hpp>
#include <cppcoro/detail/unwrap_reference.hpp>

#include <tuple>
#include <utility>
#include <vector>
#include <type_traits>

namespace cppcoro {

auto when_all_ready(std::vector<Task> awaitables) {
	std::vector<detail::WhenAllTask> tasks;

	tasks.reserve(awaitables.size());
	for (auto& awaitable : awaitables) {
		tasks.emplace_back(detail::make_when_all_task(std::move(awaitable)));
	}

	return detail::WhenAllReadyAwaitable(std::move(tasks));
}
} // namespace cppcoro
