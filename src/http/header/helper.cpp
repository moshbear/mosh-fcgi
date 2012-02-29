#include <memory>
#include <stdexcept>
#include <string>
#include <mosh/fcgi/http/header/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {
	std::runtime_error undefined_overload(std::string const& sig) {
		return std::runtime_error("http::header::Helper: undefined override for overload: " + sig);
	}

}

MOSH_FCGI_BEGIN

namespace http {

namespace header {

Helper::header_pair Helper::operator()(unsigned) {
	throw undefined_overload("(unsigned)");
}

Helper::header_pair Helper::operator()(std::string const&) {
	throw undefined_overload("(std::string const&)");
}

Helper::header_pair Helper::operator()(unsigned, unsigned) {
	throw undefined_overload("(unsigned, unsigned)");
}
	
Helper::header_pair Helper::operator()(unsigned, std::string const&) {
	throw undefined_overload("(unsigned, std::string const&)");
}

Helper::header_pair Helper::operator()(std::string const&, unsigned) {
	throw undefined_overload("(std::string const&, unsigned)");
}

Helper::header_pair Helper::operator()(std::string const&, std::string const&) {
	throw undefined_overload("(std::string const&, std::string const& )");
}
	
}

}

MOSH_FCGI_END

