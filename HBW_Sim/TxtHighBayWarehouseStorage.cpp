#include "TxtHighBayWarehouseStorage.h"

namespace ft {

TxtHighBayWarehouseStorage::TxtHighBayWarehouseStorage() 
    : filename("Data/Config.HBW.Storage.json")
{
    resetStorageState(); 
    currentPos = {-1, -1};
    nextFetchPos = {-1, -1};
}

TxtHighBayWarehouseStorage::~TxtHighBayWarehouseStorage() {
    for(int i=0;i<3;i++) {
        for(int j=0;j<3;j++) {
            if(wp[i][j]) delete wp[i][j];
        }
    }
}

bool TxtHighBayWarehouseStorage::loadStorageState() { return true; }
bool TxtHighBayWarehouseStorage::saveStorageState() { return true; }

void TxtHighBayWarehouseStorage::resetStorageState() {
    for(int i=0;i<3;i++) {
        for(int j=0;j<3;j++) {
            wp[i][j] = nullptr;
            wpc[i][j] = true;
        }
    }
    Notify();
}

bool TxtHighBayWarehouseStorage::storeContainer() {
    if (isValidPos(nextFetchPos)) {
        wp[nextFetchPos.x][nextFetchPos.y] = nullptr; 
        Notify();
        return true;
    }
    return false;
}

bool TxtHighBayWarehouseStorage::store(TxtWorkpiece _wp) {
    SPDLOG_LOGGER_DEBUG(spdlog::get("console"), "store type:%d", _wp.type);
    
    nextFetchPos = {-1, -1}; 

    // Logic: Find first empty spot (fill bottom-up, left-to-right)
    if (nextFetchPos.x == -1) {
         for(int i=0; i<3; i++) {
            for(int j=0; j<3; j++) {
                if(wp[i][j] == nullptr) {
                    nextFetchPos = {i, j};
                    goto found_store;
                }
            }
         }
         return false; 
    }
    found_store:

    if (isValidPos(nextFetchPos)) {
        wp[nextFetchPos.x][nextFetchPos.y] = new TxtWorkpiece(_wp);
        Notify();
        return true;
    }
    return false;
}

bool TxtHighBayWarehouseStorage::fetch(TxtWPType_t t) {
    SPDLOG_LOGGER_DEBUG(spdlog::get("console"), "fetch type:%d", t);
    nextFetchPos = {-1, -1};

    // FIX: Changed search order for Fetch.
    // Previously: for(int j=2; j>=0; j--) (Top-down)
    // New:        for(int j=0; j<3; j++)  (Bottom-up)
    // This ensures items stored earlier (lower indices) are fetched first,
    // matching the FIFO-like behavior requested in the example.
    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) { // Changed loop direction here
            if (wp[i][j] && wp[i][j]->type == t) {
                nextFetchPos = {i, j};
                goto found_fetch;
            }
        }
    }
    found_fetch:

    if (isValidPos(nextFetchPos)) {
        delete wp[nextFetchPos.x][nextFetchPos.y];
        wp[nextFetchPos.x][nextFetchPos.y] = nullptr;
        Notify();
        return true;
    }
    return false;
}

bool TxtHighBayWarehouseStorage::fetchContainer() {
    for(int i=0; i<3; i++) {
        for(int j=2; j>=0; j--) {
            if (wp[i][j] == nullptr) {
                nextFetchPos = {i, j};
                return true;
            }
        }
    }
    return false;
}

bool TxtHighBayWarehouseStorage::isValidPos(StoragePos2 p) {
    return (p.x >= 0 && p.x <= 2 && p.y >= 0 && p.y <= 2);
}

bool TxtHighBayWarehouseStorage::canColorBeStored(TxtWPType_t c) { return true; }

Stock_map_t TxtHighBayWarehouseStorage::getStockMap() {
    Stock_map_t map;
    return map;
}

char TxtHighBayWarehouseStorage::charType(int x, int y) {
    if (wp[x][y]) {
        if(wp[x][y]->type == WP_TYPE_WHITE) return 'W';
        if(wp[x][y]->type == WP_TYPE_RED) return 'R';
        if(wp[x][y]->type == WP_TYPE_BLUE) return 'B';
    }
    return '0';
}

void TxtHighBayWarehouseStorage::print() {
    std::cout << "\n--- Storage ---\n";
    // Print row by row (y=2 down to 0) for display
    for(int y=2; y>=0; y--) {
        for(int x=0; x<3; x++) {
            std::cout << charType(x, y) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "---------------\n";
}

}