#include "NetworkEngine.h"

#include <Logger.h>
#include <arpa/inet.h>

namespace pay {
inline int io_check(int ret, const char* call)
{
    if (ret < 0) {
        pay::Logger::SYS()->error("[io_check] {} failed: {}", call, strerror(-ret));
    }
    return ret;
}

inline int io_check_throw(int ret, const char* call)
{
    if (ret < 0) {
        const auto errMsg = fmt::format("[io_check] {} failed: {}", call, strerror(-ret));
        throw std::runtime_error(errMsg);
    }
    return ret;
}

inline int io_uring_check_cqe(io_uring_cqe* cqe)
{
    if (cqe->res < 0) {
        pay::Logger::SYS()->error("[io_uring] cqe operation failed: {}", strerror(-cqe->res));
    }
    return cqe->res;
}

inline io_uring_sqe* io_uring_check_sqe(io_uring_sqe* sqe)
{
    if (!sqe) {
        const auto errMsg = fmt::format("[io_uring] sqe operation failed, ring is full.");
        throw std::runtime_error(errMsg);
    }
    return sqe;
}
} // pay

#define IO_CHECK(call) pay::io_check((call), #call)
#define IO_CHECK_THROW(call) pay::io_check_throw((call), #call)
#define IO_URING_CHECK_SQE(sqe) pay::io_uring_check_sqe((sqe))
#define IO_URING_CHECK_CQE(cqe) pay::io_uring_check_cqe((cqe))

int setupListeningSocket(const uint16_t port, const int numConnections)
{
    int serverFd = IO_CHECK_THROW(socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP));
    pay::Logger::CON()->info("[socket] IPv6 TCP Socket created, fd: {}", serverFd);

    sockaddr_in6 serverAddr;
    memset(&serverAddr, 0, sizeof(sockaddr_in));
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_addr = in6addr_any;
    serverAddr.sin6_port = htons(port);

    char addrStr[INET6_ADDRSTRLEN];
    inet_ntop(serverAddr.sin6_family, &serverAddr.sin6_addr, addrStr, INET6_ADDRSTRLEN);
    pay::Logger::CON()->info("[socket] IPv6 Address: {}", addrStr);
    pay::Logger::CON()->info("[socket] Listening on port: {}", ntohs(serverAddr.sin6_port));

    const int enableReuse = 1;
    IO_CHECK_THROW(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)));
    pay::Logger::CON()->debug("[socket] Server socket option 'SO_REUSEADDR': {}", enableReuse);

    // Enable dual-stack to accept both IPv4 and IPv6 client connections
    const int isIPV6Only = 0;
    IO_CHECK_THROW(setsockopt(serverFd, IPPROTO_IPV6, IPV6_V6ONLY, &isIPV6Only, sizeof(int)));
    pay::Logger::CON()->debug("[socket] Server socket option 'IPV6_V6ONLY': {}", isIPV6Only);

    IO_CHECK_THROW(bind(serverFd, (const sockaddr*)&serverAddr, sizeof(sockaddr_in6)));
    IO_CHECK_THROW(listen(serverFd, numConnections));
    pay::Logger::CON()->debug("[socket] Server bind() and listen() to {} connections",
                              numConnections);

    return serverFd;
}

namespace pay {
IO_URingEngine::IO_URingEngine(int port, int maxQueueSize, int maxConnectionSize)
{
    m_serverFd = setupListeningSocket(port, maxConnectionSize);
    io_uring_queue_init(maxQueueSize, &m_ring, 0);
}

void IO_URingEngine::setReceiverConnection(ConnectionReceiverBase* receiverPtr)
{
    m_receiverPtr = receiverPtr;
}

bool IO_URingEngine::isRunning() { return m_isRunning; }

void IO_URingEngine::step()
{
    io_uring_cqe* completeEntry;
    const int waitRes = io_uring_wait_cqe(&m_ring, &completeEntry);
    if (waitRes < 0) {
        if (waitRes != -EINTR) {
            const char* errStr = strerror(-waitRes);
            Logger::SYS()->error("[io_check] io_uring_wait_cqe failed: {}", errStr);
        }
        return;
    }
    io_uring_cqe_seen(&m_ring, completeEntry);
}

void IO_URingEngine::stop()
{
    m_isRunning = false;

    pay::Logger::SYS()->info("[program] IO ring exit");
    io_uring_queue_exit(&m_ring);

    pay::Logger::CON()->info("[socket] Server shutdown");
    shutdown(m_serverFd, SHUT_RDWR);
    close(m_serverFd);
}

} // pay