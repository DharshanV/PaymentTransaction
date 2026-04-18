#pragma once

#include <stddef.h>

namespace pay {
class ConnectionSenderBase {
public:
    virtual bool postRead(int clientFd, char* buffer, size_t size, void* userData) = 0;

    virtual bool postSend(int clientFd, const char* buffer, size_t size, void* userData) = 0;

    virtual void postClose(int clientFd, void* userData) = 0;
};

class ConnectionReceiverBase {
public:
    virtual void onAccept(int clientFd, void* userData) = 0;

    virtual void onRead(int bytesRead, void* userData) = 0;

    virtual void onSend(int res, void* userData) = 0;

    virtual void onClose(int res, void* userData) = 0;

    virtual void* allocateData() = 0;

    virtual void freeData(void* userData) = 0;
};
} // pay