#ifndef HUF_EXCEPTION_H
#define HUF_EXCEPTION_H

#include <system_error>

enum class huf_errc
{
	invalid_fstream = 1,
	invalid_header_type,
	invalid_file_type,
};

namespace std {
	template <>
	struct std::is_error_code_enum<huf_errc> : true_type {};
}

class huf_error_category : public std::error_category
{
public:
	char const* name() const noexcept override
	{
		return "huf";
	}
	std::string message(int ec) const override
	{
		switch (static_cast<huf_errc>(ec)) {
		case huf_errc::invalid_fstream:
			return "Cannot open the file";
		/*case huf_errc::invalid_header_type:
			return "Invalid header format";*/
		case huf_errc::invalid_file_type:
			return "The path does not point to a file or directory";
		default:
			return "Unknown error";
		}
	}
};

const std::error_category& huf_category() noexcept;

std::error_code make_error_code(huf_errc ec) noexcept;

#endif // HUF_EXCEPTION_H