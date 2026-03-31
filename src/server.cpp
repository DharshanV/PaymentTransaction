#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
    // System [SYS], Connection [CON], Validation [VAL], Acknowledgement [ACK]
    spdlog::set_default_logger(spdlog::stdout_color_st("server"));
    spdlog::set_pattern("[%^%l%$][%n][%t]: %v");
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    return 0;
}