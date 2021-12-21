#include "huf_exception.hpp"

const std::error_category& huf_category() noexcept
{
	static huf_error_category obj;
	return obj;
}

std::error_code make_error_code(huf_errc ec) noexcept
{
	return std::error_code{ static_cast<int>(ec), huf_category() };
}