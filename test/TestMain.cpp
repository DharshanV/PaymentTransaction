#include <Logger.h>
#include <catch2/catch_session.hpp>

int main(int argc, char* argv[])
{
    pay::Logger::setupLoggers();
    return Catch::Session().run(argc, argv);
}