#pragma once
#include "ConnectionBase.h"

#include <liburing.h>

namespace pay {
class IO_URingEngine : public ConnectionSenderBase {
public:
    IO_URingEngine(int port, int maxQueueSize);

    void setReceiverConnection(ConnectionReceiverBase* receiverPtr);

    bool isRunning();

    void step();

    void stop();

    void postRead(int clientFd, char* buffer, int size) override { }

    void postSend(int clientFd) override { }

    void postClose(int clientFd) override { }

private:
    ConnectionReceiverBase* m_receiverPtr;

    io_uring m_ring;
    std::atomic<bool> m_isRunning = { true };
};
}