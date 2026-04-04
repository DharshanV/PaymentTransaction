#include "IOCheck.h"
#include "Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
    using namespace pay;
    Logger::setupLoggers();
    return 0;
}