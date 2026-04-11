#pragma once
#include "NetworkEngine.h"

#include <array>
#include <cstring>

namespace pay {
constexpr int MAX_NUM_CONNECTIONS = (1 << 5);
constexpr int NETWORK_BUFFER_SIZE = (1 << 9);

class PaymentHandler {
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

    PaymentHandler(NetworkEngineBase& networkEngine);

    void onAccept(int clientFd);

    void onRead(int clientFd, int bytesRead);

    void onSend(int clientFd);

    void onClose(int clientFd);

    const TransactionContext* getContext(int clientFd) const;

    size_t numActiveContexts() const;

private:
    TransactionContext* addContext(int clientFd);

    TransactionContext* getContextMutable(int clientFd);

    void releaseContext(int clientFd);

private:
    NetworkEngineBase& m_networkEngine;
    std::array<TransactionContext, MAX_NUM_CONNECTIONS> m_transactionContexts;
    std::array<bool, MAX_NUM_CONNECTIONS> m_isContextsInUse = { false };
};
} // pay