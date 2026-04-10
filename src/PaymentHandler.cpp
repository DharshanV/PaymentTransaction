#include "PaymentHandler.h"

namespace pay {
PaymentHandler::PaymentHandler(NetworkEngineBase& networkEngine)
    : m_networkEngine(networkEngine)
{
}

void PaymentHandler::onAccept(int clientFd)
{
    TransactionContext* contextPtr = addContext(clientFd);
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;
    context.clientFd = clientFd;
    context.currentOp = TransactionContext::Operation::READ;
    m_networkEngine.postRead(context.clientFd);
}

void PaymentHandler::onRead(int clientFd)
{
    TransactionContext* contextPtr = getContextMutable(clientFd);
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;
    context.currentOp = TransactionContext::Operation::SEND;
    m_networkEngine.postSend(context.clientFd);
}

void PaymentHandler::onSend(int clientFd)
{
    TransactionContext* contextPtr = getContextMutable(clientFd);
    if (!contextPtr) {
        return;
    }
    TransactionContext& context = *contextPtr;
    context.currentOp = TransactionContext::Operation::CLOSE;
    m_networkEngine.postClose(context.clientFd);
}

void PaymentHandler::onClose(int clientFd) { releaseContext(clientFd); }

PaymentHandler::TransactionContext* PaymentHandler::addContext(int clientFd)
{
    // If it already exists, return nullptr
    for (size_t i = 0; i < m_transactionContexts.size(); i++) {
        const TransactionContext& context = m_transactionContexts[i];
        if (m_isContextsInUse[i] && context.clientFd == clientFd) {
            return nullptr;
        }
    }

    // Acquire and return a fresh context for clientFd
    for (size_t i = 0; i < m_transactionContexts.size(); i++) {
        if (!m_isContextsInUse[i]) {
            TransactionContext& context = m_transactionContexts[i];
            context.clientFd = clientFd;
            m_isContextsInUse[i] = true;
            return &context;
        }
    }

    return nullptr;
}

const PaymentHandler::TransactionContext* PaymentHandler::getContext(int clientFd) const
{
    for (size_t i = 0; i < m_transactionContexts.size(); i++) {
        const TransactionContext& context = m_transactionContexts[i];
        if (m_isContextsInUse[i] && context.clientFd == clientFd) {
            return &context;
        }
    }
    return nullptr;
}

PaymentHandler::TransactionContext* PaymentHandler::getContextMutable(int clientFd)
{
    for (size_t i = 0; i < m_transactionContexts.size(); i++) {
        TransactionContext& context = m_transactionContexts[i];
        if (m_isContextsInUse[i] && context.clientFd == clientFd) {
            return &context;
        }
    }
    return nullptr;
}

void PaymentHandler::releaseContext(int clientFd)
{
    for (size_t i = 0; i < m_transactionContexts.size(); i++) {
        TransactionContext& context = m_transactionContexts[i];
        if (m_isContextsInUse[i] && context.clientFd == clientFd) {
            context = TransactionContext {};
            m_isContextsInUse[i] = false;
            return;
        }
    }
}

size_t PaymentHandler::numActiveContexts() const
{
    size_t total = 0;
    for (size_t i = 0; i < m_transactionContexts.size(); i++) {
        if (m_isContextsInUse[i]) {
            ++total;
        }
    }
    return total;
}
} // pay