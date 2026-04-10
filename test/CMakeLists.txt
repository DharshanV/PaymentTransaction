find_package(Catch2 REQUIRED)

set(UNIT_TEST_TARGET test-payment-transaction)
add_executable(${UNIT_TEST_TARGET}
    TestMain.cpp
    TestTransactionLogic.cpp
)
target_include_directories(${UNIT_TEST_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(${UNIT_TEST_TARGET} PRIVATE
    payment-core
    Catch2::Catch2
)