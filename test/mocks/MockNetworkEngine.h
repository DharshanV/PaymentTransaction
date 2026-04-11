#pragma once
#include <NetworkEngine.h>
#include <vector>

struct PostReadCall {
    int clientFd;
    char* buffer;
    int size;
};
struct PostSendCall {
    int clientFd;
};
struct PostCloseCall {
    int clientFd;
};

class MockNetworkEngine : public pay::ConnectionSenderBase {
public:
    void postAccept() override { postAcceptCalls++; }

    void postRead(int clientFd, char* buffer, int size) override
    {
        postReadCalls.push_back({ .clientFd = clientFd, .buffer = buffer, .size = size });
    }

    void postSend(int clientFd) override { postSendCalls.push_back({ clientFd }); }

    void postClose(int clientFd) override { postCloseCalls.push_back({ clientFd }); }

    int postAcceptCalls = 0;
    std::vector<PostReadCall> postReadCalls;
    std::vector<PostSendCall> postSendCalls;
    std::vector<PostCloseCall> postCloseCalls;
};