#include "PaymentHandler.h"

#include <Logger.h>

namespace pay {
void PaymentHandler::setSenderConnection(ConnectionSenderBase* senderPtr)
{
    m_senderPtr = senderPtr;
}

void PaymentHandler::onAccept(int res, void* data)
{
    TransactionContext* contextPtr = (TransactionContext*)data;
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;
    context.clientFd = res;
    context.currentOp = TransactionContext::Operation::READ;
    Logger::ACK()->debug("[payment] Accept client: {}", context.clientFd);

    char* buffer = context.networkBuffer.data();
    const size_t bufferSize = context.networkBuffer.size();
    m_senderPtr->postRead(context.clientFd, buffer, bufferSize, data);
}

void PaymentHandler::onRead(int res, void* data)
{
    TransactionContext* contextPtr = (TransactionContext*)data;
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;
    context.bytesRead = res;

    const std::string_view readStr(context.networkBuffer.data(), context.bytesRead);
    Logger::ACK()->debug("[payment] Client '{}' sent:\n{}", context.clientFd, readStr);

    context.currentOp = TransactionContext::Operation::SEND;
    m_senderPtr->postSend(context.clientFd, context.networkBuffer.data(), context.bytesRead, data);
}

void PaymentHandler::onSend(int res, void* data)
{
    TransactionContext* contextPtr = (TransactionContext*)data;
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;

    if (res >= 0) {
        Logger::ACK()->debug("[payment] Client '{}' did receive data", context.clientFd);
    } else {
        Logger::ACK()->warn("[payment] Client '{}' did NOT receive data", context.clientFd);
    }

    context.currentOp = TransactionContext::Operation::CLOSE;
    m_senderPtr->postClose(context.clientFd, data);
}

void PaymentHandler::onClose(int res, void* data)
{
    TransactionContext* contextPtr = (TransactionContext*)data;
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;

    if (res >= 0) {
        Logger::ACK()->debug("[payment] Client '{}' closed", context.clientFd);
    } else {
        Logger::ACK()->warn("[payment] Client '{}' FAILED to close", context.clientFd);
    }

    // Reset the context data
    context = TransactionContext {};
}

void* PaymentHandler::allocateData()
{
    void* userData = new TransactionContext();
    if (userData) {
        Logger::SYS()->debug("[memory] Allocate TransactionContext: {}", userData);
    }
    return userData;
}

void PaymentHandler::freeData(void* userData)
{
    Logger::SYS()->debug("[memory] Free TransactionContext: {}", userData);
    delete (TransactionContext*)userData;
}

} // pay