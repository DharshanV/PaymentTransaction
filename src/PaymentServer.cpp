#include "Logger.h"
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
    PaymentHandler paymentHandler(io_uringEngine);

    g_networkEngine = &io_uringEngine;
    while (io_uringEngine.isRunning()) {
        io_uringEngine.step();
    }

    return 0;
}