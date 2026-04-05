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

inline int io_uring_check(int ret, const char* call)
{
    if (ret < 0) {
        pay::Logger::SYS()->error("[io_uring] {} failed: {}", call, strerror(-ret));
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

inline bool io_uring_check_sqe(io_uring_sqe* sqe)
{
    if (!sqe) {
        pay::Logger::SYS()->error("[io_uring] io_uring_get_sqe failed: ring is full");
        return false;
    }
    return true;
}

} // pay

#define IO_CHECK(call) pay::io_check((call), #call)
#define IO_URING_CHECK(call) pay::io_uring_check((call), #call)
#define IO_URING_CHECK_SQE(sqe) pay::io_uring_check_sqe((sqe))
#define IO_URING_CHECK_CQE(cqe) pay::io_uring_check_cqe((cqe))