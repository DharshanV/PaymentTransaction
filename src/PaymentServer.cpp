#include "Logger.h"

#include "NetworkEngine.h"
#include "PaymentHandler.h"

#include <csignal>

static pay::IO_URingEngine* g_networkEngine = nullptr;

void sigintHandle(int signo)
{
    if (g_networkEngine) {
        g_networkEngine->stop();
    }
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sigintHandle);

    using namespace pay;
    Logger::setupLoggers();

    constexpr int PORT = 8080;
    constexpr int MAX_QUEUE_SIZE = 100;
    constexpr int MAX_CONNECTION_SIZE = 20;

    IO_URingEngine io_uringEngine(PORT, MAX_QUEUE_SIZE, MAX_CONNECTION_SIZE);
    PaymentHandler paymentHandler;

    paymentHandler.setSenderConnection(&io_uringEngine);
    io_uringEngine.setReceiverConnection(&paymentHandler);

    g_networkEngine = &io_uringEngine;
    while (io_uringEngine.isRunning()) {
        io_uringEngine.step();
    }

    return 0;
}