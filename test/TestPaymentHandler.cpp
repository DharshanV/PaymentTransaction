#include "mocks/MockNetworkEngine.h"

#include <PaymentHandler.h>
#include <catch2/catch_all.hpp>

TEST_CASE("Single Transaction Logic", "[payment]")
{
    using TransactionOperation = pay::PaymentHandler::TransactionContext::Operation;

    MockNetworkEngine networkEngine;
    pay::PaymentHandler paymentHandler(networkEngine);

    const int clientFd = 123;

    SECTION("payment onAccept to network postRead")
    {
        paymentHandler.onAccept(clientFd);

        const auto* ctx = paymentHandler.getContext(clientFd);
        REQUIRE(ctx != nullptr);
        REQUIRE(ctx->clientFd == clientFd);
        REQUIRE(ctx->currentOp == TransactionOperation::READ);
        REQUIRE(paymentHandler.numActiveContexts() == 1);
        REQUIRE(networkEngine.postReadCalls.size() == 1);
        REQUIRE(networkEngine.postReadCalls[0].clientFd == clientFd);
    }

    SECTION("payment onRead to network postSend")
    {
        paymentHandler.onAccept(clientFd);
        paymentHandler.onRead(clientFd);

        const auto* ctx = paymentHandler.getContext(clientFd);
        REQUIRE(ctx != nullptr);
        REQUIRE(ctx->clientFd == clientFd);
        REQUIRE(ctx->currentOp == TransactionOperation::SEND);
        REQUIRE(paymentHandler.numActiveContexts() == 1);
        REQUIRE(networkEngine.postSendCalls.size() == 1);
        REQUIRE(networkEngine.postSendCalls[0].clientFd == clientFd);
    }

    SECTION("payment onSend to network postClose")
    {
        paymentHandler.onAccept(clientFd);
        paymentHandler.onRead(clientFd);
        paymentHandler.onSend(clientFd);

        const auto* ctx = paymentHandler.getContext(clientFd);
        REQUIRE(ctx != nullptr);
        REQUIRE(ctx->clientFd == clientFd);
        REQUIRE(ctx->currentOp == TransactionOperation::CLOSE);
        REQUIRE(paymentHandler.numActiveContexts() == 1);
        REQUIRE(networkEngine.postCloseCalls.size() == 1);
        REQUIRE(networkEngine.postCloseCalls[0].clientFd == clientFd);
    }

    SECTION("payment onClose release context")
    {
        paymentHandler.onAccept(clientFd);
        paymentHandler.onRead(clientFd);
        paymentHandler.onSend(clientFd);
        paymentHandler.onClose(clientFd);

        const auto* ctx = paymentHandler.getContext(clientFd);
        REQUIRE(ctx == nullptr);
        REQUIRE(paymentHandler.numActiveContexts() == 0);
        REQUIRE(networkEngine.postCloseCalls.size() == 1);
        REQUIRE(networkEngine.postCloseCalls[0].clientFd == clientFd);
    }

    SECTION("full payment transcation")
    {
        paymentHandler.onAccept(clientFd);
        paymentHandler.onRead(clientFd);
        paymentHandler.onSend(clientFd);
        paymentHandler.onClose(clientFd);

        const auto* ctx = paymentHandler.getContext(clientFd);
        REQUIRE(ctx == nullptr);
        REQUIRE(paymentHandler.numActiveContexts() == 0);
        REQUIRE(networkEngine.postReadCalls.size() == 1);
        REQUIRE(networkEngine.postSendCalls.size() == 1);
        REQUIRE(networkEngine.postCloseCalls.size() == 1);
    }

    SECTION("context reuse after close")
    {
        paymentHandler.onAccept(clientFd);
        paymentHandler.onRead(clientFd);
        paymentHandler.onSend(clientFd);
        paymentHandler.onClose(clientFd);

        REQUIRE(paymentHandler.numActiveContexts() == 0);
        REQUIRE(paymentHandler.getContext(clientFd) == nullptr);

        // Re-accept the same client
        paymentHandler.onAccept(clientFd);

        const auto* ctx = paymentHandler.getContext(clientFd);
        REQUIRE(paymentHandler.numActiveContexts() == 1);
        REQUIRE(ctx != nullptr);
        REQUIRE(ctx->currentOp == TransactionOperation::READ);
    }

    SECTION("onAccept to onClose")
    {
        paymentHandler.onAccept(clientFd);
        REQUIRE(paymentHandler.numActiveContexts() == 1);

        paymentHandler.onClose(clientFd);
        REQUIRE(paymentHandler.numActiveContexts() == 0);

        REQUIRE(networkEngine.postReadCalls.size() == 1);
        REQUIRE(networkEngine.postSendCalls.size() == 0);
        REQUIRE(networkEngine.postCloseCalls.size() == 0);
    }

    SECTION("onAccept to onSend")
    {
        paymentHandler.onAccept(clientFd);
        paymentHandler.onSend(clientFd);

        const auto* ctx = paymentHandler.getContext(clientFd);
        REQUIRE(ctx != nullptr);
        REQUIRE(ctx->currentOp == TransactionOperation::CLOSE);
        REQUIRE(paymentHandler.numActiveContexts() == 1);
        REQUIRE(networkEngine.postReadCalls.size() == 1);
        REQUIRE(networkEngine.postCloseCalls.size() == 1);
    }
}

TEST_CASE("Multi Transaction Logic", "[payment]")
{
    using TransactionOperation = pay::PaymentHandler::TransactionContext::Operation;

    MockNetworkEngine networkEngine;
    pay::PaymentHandler paymentHandler(networkEngine);

    const int clientFd1 = 123;
    const int clientFd2 = 456;

    SECTION("two concurrent transactions do not interfere")
    {
        paymentHandler.onAccept(clientFd1);
        paymentHandler.onAccept(clientFd2);
        REQUIRE(paymentHandler.numActiveContexts() == 2);

        // clientFd1 progresses independently
        paymentHandler.onRead(clientFd1);
        const auto* ctx1 = paymentHandler.getContext(clientFd1);
        const auto* ctx2 = paymentHandler.getContext(clientFd2);
        REQUIRE(ctx1 != ctx2);
        REQUIRE(ctx1->currentOp == TransactionOperation::SEND);
        REQUIRE(ctx2->currentOp == TransactionOperation::READ);
    }
}

TEST_CASE("Transaction Edge Case", "[payment]")
{
    using TransactionOperation = pay::PaymentHandler::TransactionContext::Operation;

    MockNetworkEngine networkEngine;
    pay::PaymentHandler paymentHandler(networkEngine);

    SECTION("no prior onAccept does nothing")
    {
        paymentHandler.onRead(123);
        REQUIRE(paymentHandler.numActiveContexts() == 0);
        REQUIRE(networkEngine.postSendCalls.size() == 0);

        paymentHandler.onSend(123);
        REQUIRE(paymentHandler.numActiveContexts() == 0);
        REQUIRE(networkEngine.postCloseCalls.size() == 0);

        paymentHandler.onClose(123);
        REQUIRE(paymentHandler.numActiveContexts() == 0);
    }

    SECTION("onAccept same clientFd twice does not duplicate context")
    {
        paymentHandler.onAccept(123);
        paymentHandler.onAccept(123);
        REQUIRE(paymentHandler.numActiveContexts() == 1);
        REQUIRE(networkEngine.postReadCalls.size() == 1);
    }

    SECTION("onClose with no prior onAccept does nothing")
    {
        paymentHandler.onClose(123);
        REQUIRE(paymentHandler.numActiveContexts() == 0);
    }

    SECTION("onRead after onClose does nothing")
    {
        paymentHandler.onAccept(123);
        paymentHandler.onRead(123);
        paymentHandler.onSend(123);
        paymentHandler.onClose(123);

        // On read on closed client does nothing
        paymentHandler.onRead(123);
        REQUIRE(networkEngine.postSendCalls.size() == 1);
        REQUIRE(paymentHandler.numActiveContexts() == 0);
    }
}