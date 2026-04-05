#include "IO_URing.h"
#include "IO_Check.h"
#include "Logger.h"

#include <liburing.h>
#include <stdexcept>

namespace pay {
IO_URing& IO_URing::instance()
{
    static IO_URing instance;
    return instance;
}

void IO_URing::initialize()
{
    instance();
}

IO_URing::IO_URing()
{
    constexpr unsigned QUEUE_SIZE = (1 << 8);
    int ret = IO_URING_CHECK(io_uring_queue_init(QUEUE_SIZE, &m_ring, 0));
    if (ret < 0) {
        throw std::runtime_error("[io_uring] Failed to initialize.");
    }
    Logger::SYS()->debug("[io_uring] Initalized with queue size: {}", QUEUE_SIZE);
}

IO_URing::~IO_URing()
{
    io_uring_queue_exit(&m_ring);
    Logger::SYS()->debug("[io_uring] Tear and shutdown.");
}
} // pay