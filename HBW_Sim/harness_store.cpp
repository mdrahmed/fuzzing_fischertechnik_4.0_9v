/*
Standalone libfuzzer harness file for the store functions.
*/

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <iostream>

// --- Definitions from the provided source code ---

typedef enum
{
    WP_TYPE_NONE,
    WP_TYPE_WHITE,
    WP_TYPE_RED,
    WP_TYPE_BLUE
} TxtWPType_t;

typedef enum
{
    WP_STATE_RAW,
    WP_STATE_PROCESSED,
    WP_STATE_REJECTED
} TxtWPState_t;

class TxtWorkpiece
{
public:
    TxtWorkpiece()
        : tag_uid(""), type(WP_TYPE_NONE), state(WP_STATE_RAW) {}
    TxtWorkpiece(const TxtWorkpiece &wp)
        : tag_uid(wp.tag_uid), type(wp.type), state(wp.state) {};
    TxtWorkpiece(std::string tag_uid, TxtWPType_t type, TxtWPState_t state)
        : tag_uid(tag_uid), type(type), state(state) {}
    virtual ~TxtWorkpiece() {}

    std::string tag_uid;
    TxtWPType_t type;
    TxtWPState_t state;
};

struct StoragePos2
{
    int x, y;
};

class TxtHighBayWarehouseStorage
{
public:
    TxtWorkpiece *wp[3][3];
    StoragePos2 nextFetchPos;

    TxtHighBayWarehouseStorage()
    {
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                wp[i][j] = nullptr;
            }
        }
        nextFetchPos = {0, 0};
    }

    bool store(TxtWorkpiece _wp)
    {
        if (isValidPos(nextFetchPos))
        {
            wp[nextFetchPos.x][nextFetchPos.y] = new TxtWorkpiece(_wp);
            return true;
        }
        return false;
    }

    bool isValidPos(StoragePos2 p)
    {
        return (p.x >= 0) && (p.x <= 2) && (p.y >= 0) && (p.y <= 2);
    }

    StoragePos2 getNextStorePos() { return nextFetchPos; }
    void saveStorageState() {}
};

class TxtHighBayWarehouse
{
public:
    TxtHighBayWarehouseStorage storage;

    bool getConv(bool b) { return true; }
    bool putCR(int x, int y) { return true; }
    void moveRef() {}

    bool store(TxtWorkpiece wp);
};

bool TxtHighBayWarehouse::store(TxtWorkpiece wp)
{
    if (storage.store(wp))
    {
        StoragePos2 p = storage.getNextStorePos();
        if (p.x < 0 || p.y < 0)
            return false;
        bool r = getConv(true);
        if (!r)
            return false;
        r = putCR(p.x, p.y);
        if (!r)
            return false;
        storage.saveStorageState();
        moveRef();
        return true;
    }
    return false;
}

// --- LLVM libFuzzer Harness ---

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // We need at least 2 bytes for the enums and some data for the string
    if (size < 3)
    {
        return 0;
    }

    // 1. Extract enum values from the fuzzer input
    // TxtWPType_t has 4 values, TxtWPState_t has 3 values
    TxtWPType_t type = static_cast<TxtWPType_t>(data[0] % 4);
    TxtWPState_t state = static_cast<TxtWPState_t>(data[1] % 3);

    // 2. Extract the string for tag_uid
    // We use the rest of the data as the string
    std::string tag_uid(reinterpret_cast<const char *>(data + 2), size - 2);

    // 3. Initialize the class under test
    TxtHighBayWarehouse warehouse;

    // 4. Create the workpiece object
    TxtWorkpiece wp(tag_uid, type, state);

    // 5. Call the function under test
    warehouse.store(wp);

    return 0;
}
