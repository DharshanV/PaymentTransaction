#include "IO_Check.h"
#include "IO_URing.h"
#include "Logger.h"

#include <arpa/inet.h>
#include <atomic>
#include <csignal>
#include <netinet/in.h>
#include <sys/socket.h>

static int g_serverFd = -1;
static std::atomic<bool> g_isRunning { true };
constexpr const char* SEND_CLIENT_MESSAGE = "Hello from server!";

struct TransactionContext {
    // ==== Network Info ====
    int clientFd = -1;

    // ==== State Machine ====
    enum class Operation { ACCEPT, SEND, CLOSE };
    Operation currentOp = Operation::ACCEPT;
};

int setupListeningSocket(const uint16_t port, const int numConnections)
{
    int serverFd = IO_CHECK_THROW(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);
    pay::Logger::SYS()->info("[socket] Address: {}", inet_ntoa(serverAddr.sin_addr));
    pay::Logger::SYS()->info("[socket] Listening on port: {}...", ntohs(serverAddr.sin_port));

    IO_CHECK_THROW(bind(serverFd, (const sockaddr*)&serverAddr, sizeof(sockaddr_in)));
    IO_CHECK_THROW(listen(serverFd, numConnections));
    pay::Logger::SYS()->debug("[socket] Initialized listen() to {} connections", numConnections);

    return serverFd;
}

void submitAcceptEntry(io_uring* ring, int serverFd, sockaddr_in* clientAddr,
                       socklen_t* clientAddrLen)
{
    io_uring_sqe* submitEntry = IO_URING_CHECK_SQE(io_uring_get_sqe(ring));
    io_uring_prep_accept(submitEntry, serverFd, (sockaddr*)clientAddr, clientAddrLen, 0);

    // Construct context for new client connection
    TransactionContext* context = new TransactionContext;
    context->currentOp = TransactionContext::Operation::ACCEPT;
    io_uring_sqe_set_data(submitEntry, context);

    IO_CHECK_THROW(io_uring_submit(ring));
}

void submitSendEntry(io_uring* ring, int clientFd, TransactionContext& context)
{
    io_uring_sqe* submitEntry = IO_URING_CHECK_SQE(io_uring_get_sqe(ring));

    const void* buffer = SEND_CLIENT_MESSAGE;
    const unsigned nbytes = strlen(SEND_CLIENT_MESSAGE);
    io_uring_prep_send(submitEntry, clientFd, buffer, nbytes, 0);
    context.currentOp = TransactionContext::Operation::SEND;
    io_uring_sqe_set_data(submitEntry, &context);

    IO_CHECK_THROW(io_uring_submit(ring));
}

void submitCloseEntry(io_uring* ring, int clientFd, TransactionContext& context)
{
    io_uring_sqe* submitEntry = IO_URING_CHECK_SQE(io_uring_get_sqe(ring));

    io_uring_prep_close(submitEntry, clientFd);
    context.currentOp = TransactionContext::Operation::CLOSE;
    io_uring_sqe_set_data(submitEntry, &context);

    IO_CHECK_THROW(io_uring_submit(ring));
}

void sigintHandle(int signo) { g_isRunning = false; }

int main(int argc, char* argv[])
{
    signal(SIGINT, sigintHandle);

    using namespace pay;
    Logger::setupLoggers();
    IO_URing::initialize();

    io_uring* ring = IO_URing::instance().getRing();
    g_serverFd = setupListeningSocket(8080, 20);

    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(sockaddr_in);

    // Submit inital accept entry to start server listening for clients
    submitAcceptEntry(ring, g_serverFd, &clientAddr, &clientAddrLen);

    io_uring_cqe* completeEntry;
    while (g_isRunning) {
        const int waitRes = io_uring_wait_cqe(ring, &completeEntry);
        if (waitRes == -EINTR) {
            // g_isRunning has been set to false, exit loop gracefully.
            break;
        }
        IO_CHECK_THROW(waitRes);

        TransactionContext* contextPtr = (TransactionContext*)io_uring_cqe_get_data(completeEntry);
        if (!contextPtr) {
            throw std::runtime_error("[io_uring] Invalid transaction context on complete entry.");
        }

        if (!IO_URING_CHECK_CQE(completeEntry)) {
            Logger::CON()->warn("[socket] Completed with failure, cleaning up...");
            if (contextPtr) {
                if (contextPtr->currentOp == TransactionContext::Operation::ACCEPT) {
                    submitAcceptEntry(ring, g_serverFd, &clientAddr, &clientAddrLen);
                }
                delete contextPtr;
            }
            io_uring_cqe_seen(ring, completeEntry);
            continue;
        }

        TransactionContext& context = *contextPtr;
        switch (context.currentOp) {
        case TransactionContext::Operation::ACCEPT: {
            context.clientFd = completeEntry->res;

            const char* clientAddrStr = inet_ntoa(clientAddr.sin_addr);
            const uint16_t clientPort = ntohs(clientAddr.sin_port);
            Logger::CON()->info("[socket] Accepted client connection from: {}:{} on fd: {}",
                                clientAddrStr, clientPort, context.clientFd);
            submitSendEntry(ring, context.clientFd, context);

            // Submit a new accept entry for more clients
            submitAcceptEntry(ring, g_serverFd, &clientAddr, &clientAddrLen);
            break;
        }
        case TransactionContext::Operation::SEND:
            submitCloseEntry(ring, context.clientFd, context);
            break;
        case TransactionContext::Operation::CLOSE:
            Logger::CON()->info("[socket] Closed client connection on fd: {}", context.clientFd);
            // Since close has been completed, clean up resources.
            delete contextPtr;
            break;
        default:
            Logger::SYS()->warn("[socket] Unknown operation on fd: {}, closing.", context.clientFd);
            submitCloseEntry(ring, context.clientFd, context);
        }

        io_uring_cqe_seen(ring, completeEntry);
    }

    if (g_serverFd >= 0) {
        pay::Logger::SYS()->info("[socket] Shutting down server...");
        shutdown(g_serverFd, SHUT_RDWR);
        close(g_serverFd);
    }
    return 0;
}