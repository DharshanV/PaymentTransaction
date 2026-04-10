#pragma once
#include <NetworkEngine.h>
#include <vector>

struct PostReadCall {
    int clientFd;
};
struct PostSendCall {
    int clientFd;
};
struct PostCloseCall {
    int clientFd;
};

class MockNetworkEngine : public pay::NetworkEngineBase {
public:
    void postRead(int clientFd) override { postReadCalls.push_back({ clientFd }); }

    void postSend(int clientFd) override { postSendCalls.push_back({ clientFd }); }

    void postClose(int clientFd) override { postCloseCalls.push_back({ clientFd }); }

    std::vector<PostReadCall> postReadCalls;
    std::vector<PostSendCall> postSendCalls;
    std::vector<PostCloseCall> postCloseCalls;
};