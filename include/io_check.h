#pragma once
#include <liburing/io_uring.h>
#include <spdlog/spdlog.h>

inline int io_check(int ret, const char* call)
{
    if (ret < 0) {
        spdlog::error("[io_check] {} failed: {}", call, strerror(-ret));
    }
    return ret;
}

inline int io_uring_check(int ret, const char* call)
{
    if (ret < 0) {
        spdlog::error("[io_uring] {} failed: {}", call, strerror(-ret));
    }
    return ret;
}

inline bool io_uring_check_cqe(io_uring_cqe* cqe)
{
    if (cqe->res < 0) {
        spdlog::error("[io_uring] cqe operation failed: {}", strerror(-cqe->res));
        return false;
    }
    return true;
}

inline bool io_uring_check_sqe(io_uring_sqe* sqe)
{
    if (!sqe) {
        spdlog::error("[io_uring] io_uring_get_sqe failed: ring is full");
        return false;
    }
    return true;
}

#define IO_CHECK(call) io_check((call), #call)
#define IO_URING_CHECK(call) io_uring_check((call), #call)
#define IO_URING_CHECK_SQE(sqe) io_uring_check_sqe((sqe))
#define IO_URING_CHECK_CQE(cqe) io_uring_check_cqe((cqe))