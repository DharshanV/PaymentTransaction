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
        int bytesRead = 0;

        // ==== State Machine ====
        enum class Operation { ACCEPT, READ, SEND, CLOSE };
        Operation currentOp = Operation::ACCEPT;
    };

    void setSenderConnection(ConnectionSenderBase* senderPtr);

    void onAccept(int res, void* data) override;

    void onRead(int res, void* data) override;

    void onSend(int res, void* data) override;

    void onClose(int res, void* data) override;

    void* allocateData() override { return new TransactionContext(); }

    void freeData(void* data) override { delete (TransactionContext*)data; }

private:
    ConnectionSenderBase* m_senderPtr;
};
} // pay