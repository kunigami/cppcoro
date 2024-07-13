///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCORO_FILE_HPP_INCLUDED
#define CPPCORO_FILE_HPP_INCLUDED

#include <cppcoro/config.hpp>

#include <cppcoro/file_open_mode.hpp>
#include <cppcoro/file_share_mode.hpp>
#include <cppcoro/file_buffering_mode.hpp>

#include <filesystem>

namespace cppcoro
{
	class io_service;

	class file
	{
	public:

		file(file&& other) noexcept = default;

		virtual ~file();

		/// Get the size of the file in bytes.
		std::uint64_t size() const;

	protected:
	};
}

#endif
