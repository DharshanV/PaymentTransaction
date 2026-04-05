#include "IO_URing.h"
#include "IO_Check.h"
#include "Logger.h"

#include <liburing.h>

namespace pay {
IO_URing& IO_URing::instance()
{
    static IO_URing instance;
    return instance;
}

void IO_URing::initialize() { instance(); }

IO_URing::IO_URing()
{
    constexpr unsigned QUEUE_SIZE = (1 << 8);
    IO_CHECK_THROW(io_uring_queue_init(QUEUE_SIZE, &m_ring, 0));
    Logger::SYS()->info("[io_uring] Initialize IO queue size: {}", QUEUE_SIZE);
}

IO_URing::~IO_URing()
{
    io_uring_queue_exit(&m_ring);
    Logger::SYS()->info("[io_uring] Shutting down IO queue...");
}
} // pay