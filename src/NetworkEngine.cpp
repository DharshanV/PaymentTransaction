#include "NetworkEngine.h"

namespace pay {
IO_URingEngine::IO_URingEngine(int port, int maxQueueSize)
{
    io_uring_queue_init(maxQueueSize, &m_ring, 0);
}

void IO_URingEngine::setReceiverConnection(ConnectionReceiverBase* receiverPtr)
{
    m_receiverPtr = receiverPtr;
}

bool IO_URingEngine::isRunning() { return m_isRunning; }

void IO_URingEngine::step()
{
    io_uring_cqe* completeEntry;
    const int waitRes = io_uring_wait_cqe(&m_ring, &completeEntry);
    if (waitRes < 0) {
        return;
    }
    io_uring_cqe_seen(&m_ring, completeEntry);
}

void IO_URingEngine::stop()
{
    m_isRunning = false;
    io_uring_queue_exit(&m_ring);
}

} // pay