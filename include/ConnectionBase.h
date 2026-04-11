#pragma once

namespace pay {
class ConnectionSenderBase {
public:
    virtual void postRead(int clientFd, char* buffer, int size) = 0;

    virtual void postSend(int clientFd) = 0;

    virtual void postClose(int clientFd) = 0;
};

class ConnectionReceiverBase {
public:
    virtual void onAccept(int clientFd) = 0;

    virtual void onRead(int clientFd, int bytesRead) = 0;

    virtual void onSend(int clientFd) = 0;

    virtual void onClose(int clientFd) = 0;
};
} // pay