#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace pay {
namespace Logger {
// Setup these loggers:
// - System             [SYS]: System level logs (i.e. io_uring and socket).
// - Connection         [CON]: All connection related logs.
// - Validation         [VAL]: Payment validation logs.
// - Acknowledgement    [ACK]: Payment final result logs.
// - Log                [LOG]: General and default logs.
// Log pattern: "[%^%l%$][%n][%t]: %v"
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
}

inline std::shared_ptr<spdlog::logger> SYS() { return spdlog::get("SYS"); }
inline std::shared_ptr<spdlog::logger> CON() { return spdlog::get("CON"); }
inline std::shared_ptr<spdlog::logger> VAL() { return spdlog::get("VAL"); }
inline std::shared_ptr<spdlog::logger> ACK() { return spdlog::get("ACK"); }
inline std::shared_ptr<spdlog::logger> LOG() { return spdlog::get("LOG"); }

} // Logger
} // pay