#ifndef SRC_HTTP_MIME_HPP
#define SRC_HTTP_MIME_HPP

#include <string>
#include <src/utility.hpp>
#include <src/namespace.hpp>

SRC_BEGIN

namespace mime {

//! Content-type-encoding filtering flags
namespace cte_flags {
	const unsigned ign = 1; // trivial C-t-e
	const unsigned def = 2; // non-trivial C-t-e; additional parsing needed
}

std::string fetch_cte(std::string const& str, int cmp_flag = cte_flags::def);
inline bool match_cte(std::string const& str, int cmp_flag = cte_flags::def) {
	return !fetch_content_type(str, cmp_flag).empty();
}

// A maybe string.
namespace maybe_string {
	// If %.first == %true, %.second is resulting string;
	// if %.first == %false, %.second is the error string.
	typedef std::pair<bool, std::string> type;

	type ok(std::string const& s)
	{ return type(true, s);	}
	type ok(std::string&& s)
	{ return type(true, std::move(s)); }
	type error(std::string const& s)
	{ return type(false, s); }
	type error(std::string&& s)
	{ return type(false, std::move(s)); }
}

// unfold long lines and strip comments
maybe_string::type canonicalize(std::string const&);

std::map<std::string, std::string> get_mime_params(std::string const& header_line);

// Decode a RFC2047 encoded-word (i.e. =\?(charset)\?([BQbq])\?(encoded_text)\?=
std::wstring rfc2047_decode(std::string const& encoded_word);

}

namespace filtering {
	enum class field {
		content_type,
		content_disposition,
	};
	enum class subfield_content_type {
		boundary,
		charset,
	};
	enum class subfield_content_disposition {
		name,
		filename,
	};
	typedef typename common_enum<subfield_content_type, subfield_content_disposition>::type subfield_union;

	std::string filter(std::string const& str, field f, subfield_union subf);
}

}
	
SRC_END

#endif
