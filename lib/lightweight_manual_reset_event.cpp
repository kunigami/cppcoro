///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <cppcoro/detail/lightweight_manual_reset_event.hpp>

#include <system_error>

namespace cppcoro::detail {
lightweight_manual_reset_event::lightweight_manual_reset_event()
	: m_isSet(false)
{}

void lightweight_manual_reset_event::set() noexcept {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_isSet = true;
	m_cv.notify_all();
}

void lightweight_manual_reset_event::reset() noexcept {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_isSet = false;
}

void lightweight_manual_reset_event::wait() noexcept {
	std::unique_lock<std::mutex> lock(m_mutex);
	m_cv.wait(lock, [this] { return m_isSet; });
}
} // namespace cppcoro::detail
