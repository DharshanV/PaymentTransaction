cmake_minimum_required(VERSION 3.25)

project(PaymentTransaction LANGUAGES CXX)

find_package(Tracy REQUIRED)
find_package(spdlog REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_INIT "-Wall -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

# ==== Core Library ====
set(CORE_LIB_TARGET payment-core)
add_library(${CORE_LIB_TARGET} OBJECT 
    src/IO_URing.cpp
    src/Logger.cpp
)
target_include_directories(${CORE_LIB_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${CORE_LIB_TARGET} PUBLIC
    uring
    spdlog::spdlog
)

# ==== Server ====
set(SERVER_TARGET payment-server)
add_executable(${SERVER_TARGET} src/server.cpp)
target_link_libraries(${SERVER_TARGET} PRIVATE
    ${CORE_LIB_TARGET}
    Tracy::TracyClient
)
install(TARGETS ${SERVER_TARGET} DESTINATION bin)