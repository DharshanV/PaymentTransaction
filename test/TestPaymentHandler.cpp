#include "mocks/MockNetworkEngine.h"

#include <PaymentHandler.h>
#include <catch2/catch_all.hpp>

TEST_CASE("Single Transaction Logic", "[payment]")
{
    using TransactionOperation = pay::PaymentHandler::TransactionContext::Operation;
    using TransactionContext = pay::PaymentHandler::TransactionContext;

    MockNetworkEngine networkEngine;
    pay::PaymentHandler paymentHandler;
    paymentHandler.setSenderConnection(&networkEngine);

    const int clientFd = 123;
    const int readBytes = 10;
    TransactionContext* ctx = (TransactionContext*)paymentHandler.allocateData();

    SECTION("payment onAccept to network postRead")
    {
        paymentHandler.onAccept(clientFd, ctx);
        REQUIRE(ctx->clientFd == clientFd);
        REQUIRE(ctx->currentOp == TransactionOperation::READ);
        REQUIRE(networkEngine.postReadCalls.size() == 1);
        REQUIRE(networkEngine.postReadCalls[0].clientFd == clientFd);
        REQUIRE(networkEngine.postAcceptCalls == 1);
    }

    SECTION("payment onRead to network postSend")
    {
        paymentHandler.onAccept(clientFd, ctx);
        paymentHandler.onRead(readBytes, ctx);

        REQUIRE(ctx->clientFd == clientFd);
        REQUIRE(ctx->currentOp == TransactionOperation::SEND);
        REQUIRE(networkEngine.postSendCalls.size() == 1);
        REQUIRE(networkEngine.postSendCalls[0].clientFd == clientFd);
    }

    SECTION("payment onSend to network postClose")
    {
        paymentHandler.onAccept(clientFd, ctx);
        paymentHandler.onRead(readBytes, ctx);
        paymentHandler.onSend(0, ctx);

        REQUIRE(ctx->clientFd == clientFd);
        REQUIRE(ctx->currentOp == TransactionOperation::CLOSE);
        REQUIRE(networkEngine.postCloseCalls.size() == 1);
        REQUIRE(networkEngine.postCloseCalls[0].clientFd == clientFd);
    }

    SECTION("full payment transaction")
    {
        paymentHandler.onAccept(clientFd, ctx);
        paymentHandler.onRead(readBytes, ctx);
        paymentHandler.onSend(0, ctx);
        paymentHandler.onClose(0, ctx);

        REQUIRE(networkEngine.postReadCalls.size() == 1);
        REQUIRE(networkEngine.postSendCalls.size() == 1);
        REQUIRE(networkEngine.postCloseCalls.size() == 1);
    }

    SECTION("context reuse after close")
    {
        paymentHandler.onAccept(clientFd, ctx);
        paymentHandler.onRead(readBytes, ctx);
        paymentHandler.onSend(0, ctx);
        paymentHandler.onClose(0, ctx);

        // Re-accept the same client with a fresh context
        TransactionContext* ctx2 = (TransactionContext*)paymentHandler.allocateData();
        paymentHandler.onAccept(clientFd, ctx2);

        REQUIRE(ctx2->clientFd == clientFd);
        REQUIRE(ctx2->currentOp == TransactionOperation::READ);

        paymentHandler.freeData(ctx2);
    }

    SECTION("onAccept to onClose")
    {
        paymentHandler.onAccept(clientFd, ctx);
        paymentHandler.onClose(0, ctx);

        REQUIRE(networkEngine.postReadCalls.size() == 1);
        REQUIRE(networkEngine.postSendCalls.size() == 0);
        REQUIRE(networkEngine.postCloseCalls.size() == 0);
    }

    SECTION("onAccept to onSend")
    {
        paymentHandler.onAccept(clientFd, ctx);
        paymentHandler.onSend(0, ctx);

        REQUIRE(ctx->clientFd == clientFd);
        REQUIRE(ctx->currentOp == TransactionOperation::CLOSE);
        REQUIRE(networkEngine.postReadCalls.size() == 1);
        REQUIRE(networkEngine.postCloseCalls.size() == 1);
    }

    paymentHandler.freeData(ctx);
}

TEST_CASE("Multi Transaction Logic", "[payment]")
{
    using TransactionOperation = pay::PaymentHandler::TransactionContext::Operation;
    using TransactionContext = pay::PaymentHandler::TransactionContext;

    MockNetworkEngine networkEngine;
    pay::PaymentHandler paymentHandler;
    paymentHandler.setSenderConnection(&networkEngine);

    const int clientFd1 = 123;
    const int clientFd2 = 456;
    const int readBytes = 0;

    SECTION("two concurrent transactions do not interfere")
    {
        TransactionContext* ctx1 = (TransactionContext*)paymentHandler.allocateData();
        TransactionContext* ctx2 = (TransactionContext*)paymentHandler.allocateData();

        paymentHandler.onAccept(clientFd1, ctx1);
        paymentHandler.onAccept(clientFd2, ctx2);

        // clientFd1 progresses independently
        paymentHandler.onRead(readBytes, ctx1);

        REQUIRE(ctx1 != ctx2);
        REQUIRE(ctx1->currentOp == TransactionOperation::SEND);
        REQUIRE(ctx2->currentOp == TransactionOperation::READ);

        paymentHandler.freeData(ctx1);
        paymentHandler.freeData(ctx2);
    }
}

TEST_CASE("Transaction Edge Case", "[payment]")
{
    using TransactionOperation = pay::PaymentHandler::TransactionContext::Operation;
    using TransactionContext = pay::PaymentHandler::TransactionContext;

    MockNetworkEngine networkEngine;
    pay::PaymentHandler paymentHandler;
    paymentHandler.setSenderConnection(&networkEngine);

    SECTION("null data on callbacks does nothing")
    {
        paymentHandler.onAccept(123, nullptr);
        REQUIRE(networkEngine.postReadCalls.size() == 0);
        REQUIRE(networkEngine.postAcceptCalls == 0);

        paymentHandler.onRead(0, nullptr);
        REQUIRE(networkEngine.postSendCalls.size() == 0);

        paymentHandler.onSend(0, nullptr);
        REQUIRE(networkEngine.postCloseCalls.size() == 0);

        paymentHandler.onClose(0, nullptr);
    }

    SECTION("fresh context has default state")
    {
        TransactionContext* ctx = (TransactionContext*)paymentHandler.allocateData();

        REQUIRE(ctx->clientFd == -1);
        REQUIRE(ctx->bytesRead == 0);
        REQUIRE(ctx->currentOp == TransactionOperation::ACCEPT);

        paymentHandler.freeData(ctx);
    }
}

TEST_CASE("Transaction onRead Buffer Logic", "[payment][read]")
{
    using TransactionOperation = pay::PaymentHandler::TransactionContext::Operation;
    using TransactionContext = pay::PaymentHandler::TransactionContext;

    MockNetworkEngine networkEngine;
    pay::PaymentHandler paymentHandler;
    paymentHandler.setSenderConnection(&networkEngine);

    const int clientFd = 123;
    TransactionContext* ctx = (TransactionContext*)paymentHandler.allocateData();

    SECTION("postRead receives correct context buffer")
    {
        paymentHandler.onAccept(clientFd, ctx);

        REQUIRE(networkEngine.postReadCalls.size() == 1);
        const auto& call = networkEngine.postReadCalls[0];
        REQUIRE(call.clientFd == clientFd);
        REQUIRE(call.buffer != nullptr);
        REQUIRE(call.size == pay::NETWORK_BUFFER_SIZE);
        // zero-copy: postRead buffer points directly into context
        REQUIRE(call.buffer == ctx->networkBuffer.data());
    }

    SECTION("bytesRead stored in context after onRead")
    {
        paymentHandler.onAccept(clientFd, ctx);
        const int bytesRead = 128;
        paymentHandler.onRead(bytesRead, ctx);

        REQUIRE(ctx->clientFd == clientFd);
        REQUIRE(ctx->bytesRead == bytesRead);
    }

    SECTION("bytesRead from mock server write")
    {
        paymentHandler.onAccept(clientFd, ctx);

        const char* MOCK_SERVER_WRITE = "Hello from mock server!";

        // Simulate kernel writing into the buffer
        const auto& call = networkEngine.postReadCalls.back();
        const int bytesWrite = std::char_traits<char>::length(MOCK_SERVER_WRITE);
        memcpy(call.buffer, MOCK_SERVER_WRITE, bytesWrite);

        // Notify PaymentHandler that bytes have been read
        paymentHandler.onRead(bytesWrite, ctx);

        REQUIRE(ctx->clientFd == clientFd);
        REQUIRE(ctx->bytesRead == bytesWrite);
        REQUIRE(strcmp(MOCK_SERVER_WRITE, ctx->networkBuffer.data()) == 0);
    }

    SECTION("data not corrupted after close and new accept with different messages")
    {
        // === First connection ===
        paymentHandler.onAccept(clientFd, ctx);

        const char* FIRST_WRITE = "First payment request!";
        {
            const auto& readCall = networkEngine.postReadCalls.back();
            const int bytesWrite = std::char_traits<char>::length(FIRST_WRITE);
            memcpy(readCall.buffer, FIRST_WRITE, bytesWrite);
            paymentHandler.onRead(bytesWrite, ctx);

            REQUIRE(ctx->bytesRead == bytesWrite);
            REQUIRE(strcmp(FIRST_WRITE, ctx->networkBuffer.data()) == 0);
        }

        // Clean up before second connection
        paymentHandler.onClose(0, ctx);

        // === Second connection - same fd, different message ===
        TransactionContext* ctx2 = (TransactionContext*)paymentHandler.allocateData();
        paymentHandler.onAccept(clientFd, ctx2);
        {
            // Context is all clear
            REQUIRE(ctx2 != nullptr);
            REQUIRE(ctx2->bytesRead == 0);
            REQUIRE(ctx2->networkBuffer[0] == 0);
            REQUIRE(strcmp(FIRST_WRITE, ctx2->networkBuffer.data()) != 0);
        }

        const char* SECOND_WRITE = "Second payment request!";
        {
            const auto& readCall = networkEngine.postReadCalls.back();
            const int bytesWrite = std::char_traits<char>::length(SECOND_WRITE);
            memcpy(readCall.buffer, SECOND_WRITE, bytesWrite);
            paymentHandler.onRead(bytesWrite, ctx2);

            REQUIRE(ctx2->bytesRead == bytesWrite);
            REQUIRE(strcmp(SECOND_WRITE, ctx2->networkBuffer.data()) == 0);
            REQUIRE(strcmp(FIRST_WRITE, ctx2->networkBuffer.data()) != 0);
        }

        paymentHandler.freeData(ctx2);
    }

    paymentHandler.freeData(ctx);
}