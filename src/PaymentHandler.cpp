#include "PaymentHandler.h"

#include <Logger.h>

namespace pay {
void PaymentHandler::setSenderConnection(ConnectionSenderBase* senderPtr)
{
    m_senderPtr = senderPtr;
}

void PaymentHandler::onAccept(int res, void* userData)
{
    TransactionContext* contextPtr = (TransactionContext*)userData;
    if (!contextPtr) {
        return;
    }

    if (res < 0) {
        Logger::ACK()->warn("[payment] Accept failed: {}", strerror(-res));
        this->freeData(userData);
        return;
    }

    TransactionContext& context = *contextPtr;
    context.clientFd = res;
    Logger::ACK()->debug("[payment] Accept client: {}", context.clientFd);

    char* buffer = context.networkBuffer.data();
    const size_t bufferSize = context.networkBuffer.size();
    if (!m_senderPtr->postRead(context.clientFd, buffer, bufferSize, userData)) {
        m_senderPtr->postClose(context.clientFd, userData);
    }
}

void PaymentHandler::onRead(int res, void* userData)
{
    TransactionContext* contextPtr = (TransactionContext*)userData;
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;

    if (res <= 0) {
        if (res == 0) {
            Logger::ACK()->info("[payment] Client '{}' disconnected gracefully", context.clientFd);
        } else {
            Logger::ACK()->warn("[payment] Client '{}' read failed: {}", context.clientFd,
                                strerror(-res));
        }
        m_senderPtr->postClose(contextPtr->clientFd, userData);
        return;
    }
    context.bytesRead = res;

    const char* networkBuffer = context.networkBuffer.data();
    const std::string_view readStr(networkBuffer, context.bytesRead);
    Logger::ACK()->debug("[payment] Client '{}' sent:\n{}", context.clientFd, readStr);

    if (!m_senderPtr->postSend(context.clientFd, networkBuffer, context.bytesRead, userData)) {
        m_senderPtr->postClose(context.clientFd, userData);
    }
}

void PaymentHandler::onSend(int res, void* userData)
{
    TransactionContext* contextPtr = (TransactionContext*)userData;
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;

    if (res < 0) {
        Logger::ACK()->warn("[payment] Client '{}' send failed: {}", context.clientFd,
                            strerror(-res));
        m_senderPtr->postClose(contextPtr->clientFd, userData);
        return;
    }
    Logger::ACK()->debug("[payment] Client '{}' did receive data", context.clientFd);

    m_senderPtr->postClose(context.clientFd, userData);
}

void PaymentHandler::onClose(int res, void* data)
{
    TransactionContext* contextPtr = (TransactionContext*)data;
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;

    if (res < 0) {
        Logger::ACK()->warn("[payment] Client '{}' close failed: {}", context.clientFd,
                            strerror(-res));
    } else {
        Logger::ACK()->debug("[payment] Client '{}' closed", context.clientFd);
    }

    // Reset the context data
    context = TransactionContext {};
}

void* PaymentHandler::allocateData()
{
    void* userData = new TransactionContext();
    Logger::SYS()->debug("[memory] Allocate TransactionContext: {}", userData);
    return userData;
}

void PaymentHandler::freeData(void* userData)
{
    Logger::SYS()->debug("[memory] Free TransactionContext: {}", userData);
    delete (TransactionContext*)userData;
}

} // pay