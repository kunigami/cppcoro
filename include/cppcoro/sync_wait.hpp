///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cppcoro/detail/lightweight_manual_reset_event.hpp>
#include <cppcoro/detail/sync_wait_task.hpp>
#include <cppcoro/task.hpp>

#include <cstdint>
#include <iostream>
#include <atomic>

namespace cppcoro {
std::string sync_wait(Task&& awaitable) {
	detail::sync_wait_task task = detail::make_sync_wait_task(std::forward<Task>(awaitable));
	detail::lightweight_manual_reset_event event;
	std::cout << "sync_wait\n";
	task.start(event);
	// Block until task call event.set()
	std::cout << "Waiting for task to complete...\n";
	event.wait();
	return task.result();
}
} // namespace cppcoro
