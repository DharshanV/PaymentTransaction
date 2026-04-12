#pragma once
#include <NetworkEngine.h>
#include <vector>

struct PostReadCall {
    int clientFd;
    char* buffer;
    size_t size;
};
struct PostSendCall {
    int clientFd;
};
struct PostCloseCall {
    int clientFd;
};

class MockNetworkEngine : public pay::ConnectionSenderBase {
public:
    void postRead(int clientFd, char* buffer, size_t size, void* data) override
    {
        postReadCalls.push_back({ .clientFd = clientFd, .buffer = buffer, .size = size });
    }

    void postSend(int clientFd, const char* buffer, size_t size, void* data) override
    {
        postSendCalls.push_back({ clientFd });
    }

    void postClose(int clientFd, void* data) override { postCloseCalls.push_back({ clientFd }); }

    std::vector<PostReadCall> postReadCalls;
    std::vector<PostSendCall> postSendCalls;
    std::vector<PostCloseCall> postCloseCalls;
};