#ifndef TXTHIGHBAYWAREHOUSESTORAGE_H_
#define TXTHIGHBAYWAREHOUSESTORAGE_H_

#include "SimulationMocks.h"
#include <map>

namespace ft {

struct StoragePos2 { int x, y; };

class TxtHighBayWarehouseStorage : public SubjectObserver {
public:
    TxtHighBayWarehouseStorage();
    virtual ~TxtHighBayWarehouseStorage();

    bool loadStorageState();
    bool saveStorageState();
    void resetStorageState();

    bool store(TxtWorkpiece _wp);
    bool storeContainer();
    bool fetch(TxtWPType_t t);
    bool fetchContainer();

    StoragePos2 getNextStorePos() { return nextFetchPos; } 
    StoragePos2 getNextFetchPos() { return nextFetchPos; }
    StoragePos2 getCurrentPos() { return currentPos; }

    bool isValidPos(StoragePos2 p);
    bool canColorBeStored(TxtWPType_t c);
    Stock_map_t getStockMap();
    void print(); 

protected:
    std::string filename;
    char charType(int x, int y);

    TxtWorkpiece * wp[3][3]; 
    bool wpc[3][3]; 

    StoragePos2 currentPos;
    StoragePos2 nextFetchPos;
};

class TxtHighBayWarehouseStorageObserver : public ft::Observer {
public:
    TxtHighBayWarehouseStorageObserver(ft::TxtHighBayWarehouseStorage* s, ft::TxtMqttFactoryClient* mqttclient)
        : _subject(s), _mqttclient(mqttclient) { _subject->Attach(this); }
    virtual ~TxtHighBayWarehouseStorageObserver() { _subject->Detach(this); }
    void Update(ft::SubjectObserver* theChangedSubject) {
        if(theChangedSubject == _subject) {
            // _mqttclient->publishStock(...) mocked
        }
    }
private:
    ft::TxtHighBayWarehouseStorage *_subject;
    ft::TxtMqttFactoryClient* _mqttclient;
};

} 
#endif