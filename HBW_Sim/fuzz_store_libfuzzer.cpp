#include <cstdint>
#include <cstddef>
#include <string>

#include "TxtHighBayWarehouse.h"

// Helper: safely map byte → enum
static ft::TxtWPType_t getType(uint8_t v)
{
    return static_cast<ft::TxtWPType_t>(v % 4);
}

static ft::TxtWPState_t getState(uint8_t v)
{
    return static_cast<ft::TxtWPState_t>(v % 3);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 3)
        return 0;

    // --- Parse input ---
    uint8_t type_raw = data[0];
    uint8_t state_raw = data[1];
    uint8_t uid_len = data[2] % 32; // limit size

    if (3 + uid_len > size)
        return 0;

    std::string uid(reinterpret_cast<const char *>(data + 3), uid_len);

    ft::TxtWPType_t type = getType(type_raw);
    ft::TxtWPState_t state = getState(state_raw);

    ft::TxtWorkpiece wp(uid, type, state);

    // --- Setup environment (mocked hardware) ---
    ft::TxtTransfer transfer;
    ft::TxtMqttFactoryClient mqtt;

    ft::TxtHighBayWarehouse hbw(&transfer, &mqtt);

    // IMPORTANT: ensure valid storage position
    // Otherwise store() will early-return always
    auto storage = hbw.getStorage();
    storage->resetStorageState();

    // Force a valid nextFetchPos (critical for coverage)
    // Otherwise isValidPos() fails → no deep execution
    storage->fetchContainer(); // initializes nextFetchPos

    // --- Function under test ---
    hbw.store(wp);

    return 0;
}