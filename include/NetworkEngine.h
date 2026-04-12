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

    void postAccept();

    bool postRead(int clientFd, char* buffer, size_t size, void* userData) override;

    bool postSend(int clientFd, const char* buffer, size_t size, void* userData) override;

    void postClose(int clientFd, void* userData) override;

private:
    struct SubmitEntryData {
        enum class Operation { ACCEPT, READ, SEND, CLOSE };

        sockaddr_storage clientAddr = {};
        socklen_t clientAddrLen = 0;
        Operation operation = Operation::ACCEPT;
        void* userData = nullptr;
    };

    io_uring m_ring;
    int m_serverFd;
    std::atomic<bool> m_isRunning = { true };

    ConnectionReceiverBase* m_receiverPtr;
};
}