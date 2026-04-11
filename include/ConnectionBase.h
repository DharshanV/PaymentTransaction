#pragma once

namespace pay {
class ConnectionSenderBase {
public:
    virtual void postAccept() = 0;

    virtual void postRead(int clientFd, char* buffer, int size) = 0;

    virtual void postSend(int clientFd) = 0;

    virtual void postClose(int clientFd) = 0;
};

class ConnectionReceiverBase {
public:
    virtual void onAccept(int res, void* data) = 0;

    virtual void onRead(int res, void* data) = 0;

    virtual void onSend(int res, void* data) = 0;

    virtual void onClose(int res, void* data) = 0;

    virtual void* allocateData() = 0;

    virtual void freeData(void* data) = 0;
};
} // pay