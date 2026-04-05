#include "IO_URing.h"
#include "Logger.h"

#include <csignal>

void sigintHandle(int signo)
{
    pay::Logger::SYS()->info("SIGINT received. Shutting down...");
    exit(0);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sigintHandle);

    using namespace pay;
    Logger::setupLoggers();
    IO_URing::initialize();
    return 0;
}