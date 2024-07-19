///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#pragma once

# include <mutex>
# include <condition_variable>

namespace cppcoro::detail {
class lightweight_manual_reset_event {
public:

	lightweight_manual_reset_event();

	~lightweight_manual_reset_event() = default;

	void set() noexcept;

	void reset() noexcept;

	void wait() noexcept;

private:

	std::mutex m_mutex;
	std::condition_variable m_cv;
	bool m_isSet;
};
} // namespace cppcoro::detail
