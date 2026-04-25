find_package(Catch2 REQUIRED)

set(TEST_TARGET test-payment-transaction)
add_executable(${TEST_TARGET}
    TestMain.cpp
    TestPaymentHandler.cpp
    TestISO-8583.cpp
)
target_include_directories(${TEST_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(${TEST_TARGET} PRIVATE
    payment-core
    Catch2::Catch2
)