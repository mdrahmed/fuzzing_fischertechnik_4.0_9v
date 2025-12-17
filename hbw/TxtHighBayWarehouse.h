#ifndef TxtHighBayWarehouse_H_
#define TxtHighBayWarehouse_H_

#include "SimulationMocks.h"
#include "TxtHighBayWarehouseStorage.h"

// FSM Helper Macros
#define FSM_DECLARE_STATE_XE( stateName, attr... ) stateName
#define HBW_FETCHED 1
#define HBW_CALIB_NAV 2
#define HBW_CALIB_END 3
#define HBW_EXIT 4

namespace ft {

class TxtHighBayWarehouseObserver;

typedef enum {
    HBWCALIB_CV = 0, HBWCALIB_A1, HBWCALIB_B2, HBWCALIB_C3, HBWCALIB_END
} TxtHbwCalibPos_t;

inline const char * toString(TxtHbwCalibPos_t v) {
    switch(v) {
    case HBWCALIB_CV: return "CV";
    case HBWCALIB_A1: return "A1";
    case HBWCALIB_B2: return "B2";
    case HBWCALIB_C3: return "B3";
    default: return "";
    }
}

class TxtHighBayWarehouseCalibData : public ft::TxtCalibData {
public:
    TxtHighBayWarehouseCalibData() : TxtCalibData("Data/Calib.HBW.json") {};
    bool load();
    bool saveDefault();
    bool save();
    uint16_t hbx[3];
    uint16_t hby[3];
    EncPos2 conv;
};

class TxtHighBayWarehouse : public TxtSimulationModel {
public:
    const int ydelta = 40;

    enum State_t {
        __NO_STATE,
        FSM_DECLARE_STATE_XE( IDLE ),
        FSM_DECLARE_STATE_XE( INIT ),
        FSM_DECLARE_STATE_XE( FAULT ),
        FSM_DECLARE_STATE_XE( FETCH_CONTAINER ),
        FSM_DECLARE_STATE_XE( STORE_WP ),
        FSM_DECLARE_STATE_XE( FETCH_WP ),
        FSM_DECLARE_STATE_XE( FETCH_WP_WAIT ),
        FSM_DECLARE_STATE_XE( STORE_CONTAINER ),
        FSM_DECLARE_STATE_XE( CALIB_HBW ),
        FSM_DECLARE_STATE_XE( CALIB_HBW_NAV ),
        FSM_DECLARE_STATE_XE( CALIB_HBW_MOVE ),
    };

    const char * toString(State_t state);
    void printEntryState(State_t state) { std::cout << "Enter State: " << toString(state) << std::endl; }
    void printState(State_t state) {} 

    TxtHighBayWarehouse(TxtTransfer* pT, ft::TxtMqttFactoryClient* mqttclient = 0);
    virtual ~TxtHighBayWarehouse();

    void requestQuit() { reqQuit= true; }
    void requestVGRfetchContainer(TxtWorkpiece* wp) { reqVGRwp = wp; reqVGRfetchContainer= true; }
    void requestVGRstore(TxtWorkpiece* wp) { reqVGRwp = wp; reqVGRstore= true; }
    void requestVGRfetch(TxtWorkpiece* wp) { reqVGRwp = wp; reqVGRfetch= true; }
    void requestVGRstoreContainer(TxtWorkpiece* wp) { reqVGRwp = wp; reqVGRstoreContainer= true; }
    void requestVGRcalib() { reqVGRcalib= true; }
    void requestVGRresetStorage() { reqVGRresetStorage= true; }
    void requestJoyBut(TxtJoysticksData jd) { joyData = jd; reqJoyData = true; }

    bool loadCalib();
    void stop();
    void moveRef();
    EncPos2 getPos2();
    void moveJoystick();

    bool store(TxtWorkpiece wp);
    bool storeContainer();
    bool fetch(TxtWPType_t t);
    bool fetchContainer();
    bool canColorBeStored(TxtWPType_t c);
    void setSpeed(int16_t s);

    TxtHighBayWarehouseStorage* getStorage() { return &storage; }
    void publishStorage() { storage.Notify(); }

    void run(); // Main loop

    TxtAxis1RefSwitch axisX;
    TxtAxis1RefSwitch axisY;
    TxtAxisNSwitch axisZ;

protected:
    State_t currentState;
    State_t newState;
    TxtHbwCalibPos_t calibPos;
    EncPos2 lastPos2;

    EncPos2 moveConv(bool stop = false);
    EncPos2 moveCR(int i, int j);

    bool getCR(int i, int j);
    bool putCR(int i, int j);
    bool getConv(bool stop = false);
    bool putConv(bool stop = false);
    void moveCalibPos();
    void fsmStep();

    TxtConveyorBeltLightBarriers convBelt;
    TxtHighBayWarehouseStorage storage;
    TxtHighBayWarehouseCalibData calibData;

    bool reqQuit;
    TxtWorkpiece* reqVGRwp;
    bool reqVGRfetchContainer = false;
    bool reqVGRstore = false;
    bool reqVGRfetch = false;
    bool reqVGRstoreContainer = false;
    bool reqVGRcalib = false;
    bool reqVGRresetStorage = false;
    TxtJoysticksData joyData;
    bool reqJoyData = false;

    TxtHighBayWarehouseObserver* obs_hbw;
    TxtHighBayWarehouseStorageObserver* obs_storage;
    TxtMqttFactoryClient* mqttclient;
};

class TxtHighBayWarehouseObserver : public ft::Observer {
public:
    TxtHighBayWarehouseObserver(ft::TxtHighBayWarehouse* s, ft::TxtMqttFactoryClient* mqttclient)
        : _subject(s), _mqttclient(mqttclient) { _subject->Attach(this); }
    virtual ~TxtHighBayWarehouseObserver() { _subject->Detach(this); }
    void Update(ft::SubjectObserver* theChangedSubject) {
        if(theChangedSubject == _subject) {
            _mqttclient->publishStateHBW(LEDS_READY,"",100,_subject->isActive()?1:0,"");
        }
    }
private:
    ft::TxtHighBayWarehouse *_subject;
    ft::TxtMqttFactoryClient* _mqttclient;
};

} /* namespace ft */

#endif