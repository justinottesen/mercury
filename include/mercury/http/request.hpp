#pragma once

#include <utility>
#include <vector>

#include <mercury/http/method.hpp>
#include <mercury/http/version.hpp>
#include <mercury/net/slice.hpp>

namespace mercury::http {

struct Request {
    Method                                                  method;
    net::Slice                                              target;
    Version                                                 version;
    std::pmr::vector<std::pair<net::Slice, net::Slice>>     headers;
};

}    // namespace mercury::http
