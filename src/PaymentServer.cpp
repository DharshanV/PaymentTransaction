#include "Logger.h"
#include "PaymentHandler.h"

#include <csignal>

static std::atomic<bool> g_isRunning { true };

void sigintHandle(int signo) { g_isRunning = false; }

int main(int argc, char* argv[])
{
    signal(SIGINT, sigintHandle);

    using namespace pay;
    Logger::setupLoggers();

    IO_URingEngine io_uringEngine;
    PaymentHandler paymentHandler(io_uringEngine);
    return 0;
}