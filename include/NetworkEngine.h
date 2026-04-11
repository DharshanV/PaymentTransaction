#pragma once
#include <liburing.h>

namespace pay {
class NetworkEngineBase {
public:
    virtual void postRead(int clientFd, char* buffer, int size) = 0;

    virtual void postSend(int clientFd) = 0;

    virtual void postClose(int clientFd) = 0;
};

class IO_URingEngine : public NetworkEngineBase {
public:
    IO_URingEngine(int port, int maxQueueSize);

    bool isRunning();

    void step();

    void stop();

    void postRead(int clientFd, char* buffer, int size) override { }

    void postSend(int clientFd) override { }

    void postClose(int clientFd) override { }

private:
    io_uring m_ring;
    std::atomic<bool> m_isRunning = { true };
};
}