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

    IO_URingEngine io_uringEngine(8080, 100);
    PaymentHandler paymentHandler;

    paymentHandler.setSenderConnection(&io_uringEngine);
    io_uringEngine.setReceiverConnection(&paymentHandler);

    g_networkEngine = &io_uringEngine;
    while (io_uringEngine.isRunning()) {
        io_uringEngine.step();
    }

    return 0;
}