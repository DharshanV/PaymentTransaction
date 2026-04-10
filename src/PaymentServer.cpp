#include "Logger.h"

#include <csignal>

static std::atomic<bool> g_isRunning { true };

void sigintHandle(int signo) { g_isRunning = false; }

int main(int argc, char* argv[])
{
    signal(SIGINT, sigintHandle);

    using namespace pay;
    Logger::setupLoggers();
    Logger::SYS()->debug("Hello World!");
    return 0;
}