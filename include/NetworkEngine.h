#pragma once
#include "ConnectionBase.h"

#include <liburing.h>

namespace pay {
class IO_URingEngine : public ConnectionSenderBase {
public:
    IO_URingEngine(int port, int maxQueueSize, int maxConnectionSize);

    void setReceiverConnection(ConnectionReceiverBase* receiverPtr);

    bool isRunning();

    void step();

    void stop();

    void postAccept() override;

    void postRead(int clientFd, char* buffer, int size) override;

    void postSend(int clientFd) override;

    void postClose(int clientFd) override;

private:
    io_uring m_ring;
    int m_serverFd;
    std::atomic<bool> m_isRunning = { true };

    ConnectionReceiverBase* m_receiverPtr;

    // Temporary storage to for accept request
    sockaddr_storage m_currClientAddr = {};
    socklen_t m_currClientAddrLen = sizeof(sockaddr_storage);
};
}