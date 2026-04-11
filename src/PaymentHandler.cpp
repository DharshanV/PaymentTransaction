#include "PaymentHandler.h"

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

    char* buffer = context.networkBuffer.data();
    const int bufferSize = context.networkBuffer.size();
    m_senderPtr->postRead(context.clientFd, buffer, bufferSize);

    // Sent accept to handle more client
    m_senderPtr->postAccept();
}

void PaymentHandler::onRead(int res, void* data)
{
    TransactionContext* contextPtr = (TransactionContext*)data;
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;
    context.bytesRead = res;

    context.currentOp = TransactionContext::Operation::SEND;
    m_senderPtr->postSend(context.clientFd);
}

void PaymentHandler::onSend(int res, void* data)
{
    TransactionContext* contextPtr = (TransactionContext*)data;
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;
    context.currentOp = TransactionContext::Operation::CLOSE;
    m_senderPtr->postClose(context.clientFd);
}

void PaymentHandler::onClose(int res, void* data) { }

} // pay