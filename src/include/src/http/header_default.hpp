#ifndef SRC_HTTP_HEADER_DEFAULT_HPP
#define SRC_HTTP_HEADER_DEFAULT_HPP

#include <memory>
#include <mosh/fcgi/http/header/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/namespace.hpp>

SRC_BEGIN

namespace http {

namespace header {

typedef std::shared_ptr<MOSH_FCGI::http::header::Helper> Helper_smartptr;

Helper_smartptr content_type();

Helper_smartptr redirect();

Helper_smartptr response();

Helper_smartptr status();

Helper_smartptr x_sendfile();

}

}

SRC_END

#endif
