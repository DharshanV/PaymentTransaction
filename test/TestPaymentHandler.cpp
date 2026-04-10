#include "mocks/MockNetworkEngine.h"

#include <PaymentHandler.h>
#include <catch2/catch_all.hpp>

TEST_CASE("Transaction Logic", "[payment]")
{
    MockNetworkEngine networkEngine;
    pay::PaymentHandler paymentHandler(networkEngine);
}