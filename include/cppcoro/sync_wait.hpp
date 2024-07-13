///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cppcoro/detail/lightweight_manual_reset_event.hpp>
#include <cppcoro/detail/sync_wait_task.hpp>
#include <cppcoro/awaitable_traits.hpp>
#include <cppcoro/task.hpp>

#include <cstdint>
#include <atomic>

namespace cppcoro {
std::string sync_wait(Task&& awaitable) {
	auto task = detail::make_sync_wait_task(std::forward<Task>(awaitable));
	detail::lightweight_manual_reset_event event;
	task.start(event);
	event.wait();
	return task.result();
}
} // namespace cppcoro
