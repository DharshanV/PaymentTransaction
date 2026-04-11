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

    void onAccept(int clientFd) override;

    void onRead(int clientFd, int bytesRead) override;

    void onSend(int clientFd) override;

    void onClose(int clientFd) override;

    const TransactionContext* getContext(int clientFd) const;

    size_t numActiveContexts() const;

private:
    TransactionContext* addContext(int clientFd);

    TransactionContext* getContextMutable(int clientFd);

    void releaseContext(int clientFd);

private:
    ConnectionSenderBase* m_senderPtr;

    std::array<TransactionContext, MAX_CONNECTION_SIZE> m_transactionContexts;
    std::array<bool, MAX_CONNECTION_SIZE> m_isContextsInUse = { false };
};
} // pay