#pragma once
#include "Logger.h"

#include <liburing/io_uring.h>

namespace pay {
inline int io_check(int ret, const char* call)
{
    if (ret < 0) {
        pay::Logger::SYS()->error("[io_check] {} failed: {}", call, strerror(-ret));
    }
    return ret;
}

inline int io_check_throw(int ret, const char* call)
{
    if (ret < 0) {
        const auto errMsg = fmt::format("[io_check] {} failed: {}", call, strerror(-ret));
        throw std::runtime_error(errMsg);
    }
    return ret;
}

inline bool io_uring_check_cqe(io_uring_cqe* cqe)
{
    if (cqe->res < 0) {
        pay::Logger::SYS()->error("[io_uring] cqe operation failed: {}", strerror(-cqe->res));
        return false;
    }
    return true;
}

inline io_uring_sqe* io_uring_check_sqe(io_uring_sqe* sqe)
{
    if (!sqe) {
        const auto errMsg = fmt::format("[io_uring] sqe operation failed, ring is full.");
        throw std::runtime_error(errMsg);
    }
    return sqe;
}

} // pay

#define IO_CHECK(call) pay::io_check((call), #call)
#define IO_CHECK_THROW(call) pay::io_check_throw((call), #call)
#define IO_URING_CHECK_SQE(sqe) pay::io_uring_check_sqe((sqe))
#define IO_URING_CHECK_CQE(cqe) pay::io_uring_check_cqe((cqe))