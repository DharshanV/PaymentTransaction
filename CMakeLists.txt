cmake_minimum_required(VERSION 3.25)

project(PaymentTransaction LANGUAGES CXX)

find_package(Tracy REQUIRED)
find_package(spdlog REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_INIT "-Wall -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

# ==== Server ====
set(SERVER_TARGET payment-server)
add_executable(${SERVER_TARGET} src/server.cpp)
target_include_directories(${SERVER_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${SERVER_TARGET} PRIVATE 
    uring
    Tracy::TracyClient
    spdlog::spdlog
)

install(TARGETS ${SERVER_TARGET} DESTINATION bin)