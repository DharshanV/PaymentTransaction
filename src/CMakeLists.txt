# ==== Core Library ====
set(CORE_LIB_TARGET payment-core)
add_library(${CORE_LIB_TARGET} OBJECT 
    Logger.cpp
    PaymentHandler.cpp
    NetworkEngine.cpp
)
target_include_directories(${CORE_LIB_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(${CORE_LIB_TARGET} PUBLIC
    uring
    spdlog::spdlog
)

# ==== Server ====
set(SERVER_TARGET payment-server)
add_executable(${SERVER_TARGET} PaymentServer.cpp)
target_link_libraries(${SERVER_TARGET} PRIVATE
    ${CORE_LIB_TARGET}
    Tracy::TracyClient
)
install(TARGETS ${SERVER_TARGET} DESTINATION bin)