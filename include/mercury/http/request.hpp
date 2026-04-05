#pragma once

#include <string>
#include <utility>
#include <vector>

#include <mercury/http/method.hpp>
#include <mercury/http/version.hpp>

namespace mercury::http {

struct Request {
    Method                                           method;
    std::string                                      target;
    Version                                          version;
    std::vector<std::pair<std::string, std::string>> headers;
};

}    // namespace mercury::http
