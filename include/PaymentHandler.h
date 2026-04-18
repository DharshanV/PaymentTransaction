#pragma once
#include "ConnectionBase.h"
#include "Constants.h"

#include <array>
#include <cstring>

namespace pay {
class PaymentHandler : public ConnectionReceiverBase {
public:
    struct TransactionContext {
        // ==== Network Info ====
        int clientFd = -1;
        std::array<char, NETWORK_BUFFER_SIZE> networkBuffer = { 0 };
        size_t bytesRead = 0;
    };

    void setSenderConnection(ConnectionSenderBase* senderPtr);

    void onAccept(int clientFd, void* userData) override;

    void onRead(int bytesRead, void* userData) override;

    void onSend(int res, void* userData) override;

    void onClose(int res, void* userData) override;

    void* allocateData() override;

    void freeData(void* userData) override;

private:
    ConnectionSenderBase* m_senderPtr;
};
} // pay