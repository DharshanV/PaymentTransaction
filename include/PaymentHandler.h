#pragma once
#include "NetworkEngine.h"

namespace pay {
class PaymentHandler {
public:
    PaymentHandler(NetworkEngineBase& networkEngine)
        : m_networkEngine(networkEngine)
    {
    }

private:
    NetworkEngineBase& m_networkEngine;
};
} // pay