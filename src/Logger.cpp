#include "Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace pay {
namespace Logger {
void enableLogger(std::shared_ptr<spdlog::logger> logger)
{
#ifndef NDEBUG
    logger->set_level(spdlog::level::debug);
#else
    logger->set_level(spdlog::level::warn);
#endif
}

void setupLoggers()
{
    auto systemLogger = spdlog::stdout_color_mt("SYS");
    auto connectionLogger = spdlog::stdout_color_mt("CON");
    auto validationLogger = spdlog::stdout_color_mt("VAL");
    auto ackLogger = spdlog::stdout_color_mt("ACK");
    auto logLogger = spdlog::stdout_color_mt("LOG");

    spdlog::set_default_logger(logLogger);
    spdlog::set_pattern("[%n][%^%l%$]: %v");
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    // disable all loggers
    spdlog::apply_all(
        [](std::shared_ptr<spdlog::logger> logger) { logger->set_level(spdlog::level::off); });

    enableLogger(systemLogger);
    enableLogger(connectionLogger);
    enableLogger(validationLogger);
    enableLogger(ackLogger);
    enableLogger(logLogger);
}
} // Logger
} // pay