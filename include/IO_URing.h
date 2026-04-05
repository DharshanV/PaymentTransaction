#pragma once
#include <liburing.h>

namespace pay {
class IO_URing {
public:
    static void initialize();

    static IO_URing& instance();

    IO_URing(const IO_URing&) = delete;
    IO_URing(IO_URing&&) = delete;
    IO_URing& operator=(const IO_URing&) = delete;
    IO_URing& operator=(IO_URing&&) = delete;

    io_uring* getRing() { return &m_ring; }

private:
    IO_URing();

    ~IO_URing();

private:
    io_uring m_ring;
};
} // pay