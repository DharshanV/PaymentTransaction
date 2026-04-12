#pragma once

#include <stddef.h>

namespace pay {
class ConnectionSenderBase {
public:
    virtual void postRead(int clientFd, char* buffer, size_t size, void* data) = 0;

    virtual void postSend(int clientFd, const char* buffer, size_t size, void* data) = 0;

    virtual void postClose(int clientFd, void* data) = 0;
};

class ConnectionReceiverBase {
public:
    virtual void onAccept(int res, void* data) = 0;

    virtual void onRead(int res, void* data) = 0;

    virtual void onSend(int res, void* data) = 0;

    virtual void onClose(int res, void* data) = 0;

    virtual void* allocateData() = 0;

    virtual void freeData(void* userData) = 0;
};
} // pay