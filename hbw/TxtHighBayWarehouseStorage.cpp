#include "TxtHighBayWarehouseStorage.h"

namespace ft {

TxtHighBayWarehouseStorage::TxtHighBayWarehouseStorage() {
    // Force reset instead of loading file
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
        print();
        return true;
    }
    return false;
}

bool TxtHighBayWarehouseStorage::store(TxtWorkpiece _wp) {
    // Simple mock logic: Find first empty spot if not set
    if (nextFetchPos.x == -1) {
         for(int i=0; i<3; i++) {
            for(int j=0; j<3; j++) {
                if (wp[i][j] == nullptr) {
                    nextFetchPos = {i, j};
                    goto found_store;
                }
            }
         }
         return false; // full
    }
    found_store:

    if (isValidPos(nextFetchPos)) {
        wp[nextFetchPos.x][nextFetchPos.y] = new TxtWorkpiece(_wp);
        Notify();
        print();
        return true;
    }
    return false;
}

bool TxtHighBayWarehouseStorage::fetch(TxtWPType_t t) {
    nextFetchPos = {-1, -1};
    for(int i=0; i<3; i++) {
        for(int j=2; j>=0; j--) {
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
        print();
        return true;
    }
    return false;
}

bool TxtHighBayWarehouseStorage::fetchContainer() {
    // Find empty slot (container only)
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
    return '_';
}

void TxtHighBayWarehouseStorage::print() {
    std::cout << "--- Storage Map ---\n";
    std::cout << charType(0,2) << " " << charType(1,2) << " " << charType(2,2) << "\n";
    std::cout << charType(0,1) << " " << charType(1,1) << " " << charType(2,1) << "\n";
    std::cout << charType(0,0) << " " << charType(1,0) << " " << charType(2,0) << "\n";
}

}