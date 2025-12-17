#ifndef TXTVACUUMGRIPPERROBOT_H_
#define TXTVACUUMGRIPPERROBOT_H_

#include "SimulationMocks.h"
#include "TxtVacuumGripper.h"
#include "TxtNfcDevice.h"

// FSM Helper
#define FSM_DECLARE_STATE_XE( stateName, attr... ) stateName

namespace ft {

// --- Factory Process Storage Mock ---
class TxtFactoryProcessStorage {
public:
    void setTimestampNow(std::string, TxtHistoryIndex_t) {}
    std::vector<int64_t> getTagUidVts(std::string) { return std::vector<int64_t>(); }
    uint8_t getTagUidMaskTs(std::string) { return 0; }
    void resetTagUidMaskTs(std::string) {}
};

// --- Delivery Pickup Station Mock ---
class TxtDeliveryPickupStation : public TxtSimulationModel {
public:
    TxtDeliveryPickupStation(TxtTransfer* t, TxtMqttFactoryClient* m) : TxtSimulationModel(t,m) {}
    void startThread() {}
    void Notify() {}
    void setActiveDSI(bool) {}
    void setActiveDSO(bool) {}
    void setErrorDSI(bool) {}
    void setErrorDSO(bool) {}
    bool is_DIN() { return false; } // Sim: always false (triggered)
    bool is_DOUT() { return true; } // Sim: always empty
    std::string getUIDResetHBW() { return ""; }
    std::string getUIDCalibMode() { return ""; }
    void saveUIDResetHBW(std::string) {}
    void saveUIDCalibMode(std::string) {}
    
    // NFC Relay
    TxtNfcDevice nfc;
    TxtNfcDevice* getNfc() { return &nfc; }
    bool nfcDelete() { return nfc.eraseTags(); }
    std::string nfcRead() { return nfc.readTags(); }
    std::string nfcReadUID() { return nfc.readTagsGetUID(); }
    void publishNfc() { nfc.publish(); }
    
    std::string nfcDeviceDeleteWriteRawRead(TxtWPType_t c, std::vector<int64_t>, uint8_t) {
        nfcDelete();
        TxtWorkpiece wp("TAG_NEW", c, WP_STATE_RAW); // Sim logic
        return "TAG_NEW";
    }
    
    std::string nfcDeviceWriteProducedRead(TxtWPType_t c, std::vector<int64_t>, uint8_t) {
        return "TAG_PRODUCED";
    }
    std::string nfcDeviceWriteRejectedRead(TxtWPType_t, std::vector<int64_t>, uint8_t) { return "TAG_REJECT"; }

    int readColorValue() { return 500; }
    TxtWPType_t getLastColor(); // Implemented in CPP
    TxtCalibData calibData{"file"}; 
};

class TxtDeliveryPickupStationObserver : public Observer {
public:
    TxtDeliveryPickupStationObserver(TxtDeliveryPickupStation*, TxtMqttFactoryClient*){}
    void Update(SubjectObserver*) {}
};

// --- Main VGR Class ---
class TxtVacuumGripperRobotObserver;

typedef enum { VGRMOV_PTP, VGRMOV_XYZ, VGRMOV_XZY, VGRMOV_YXZ, VGRMOV_YZX, VGRMOV_ZXY, VGRMOV_ZYX, VGRMOV_X_PTP, VGRMOV_Y_PTP, VGRMOV_Z_PTP } TxtVgrPosOrder_t;
typedef enum { VGRCALIB_DSI=0, VGRCALIB_DCS, VGRCALIB_NFC, VGRCALIB_WDC, VGRCALIB_DSO, VGRCALIB_HBW, VGRCALIB_MPO, VGRCALIB_SL1, VGRCALIB_SL2, VGRCALIB_SL3, VGRCALIB_END } TxtVgrCalibPos_t;

class TxtVacuumGripperRobotCalibData : public TxtCalibData {
public:
    TxtVacuumGripperRobotCalibData() : TxtCalibData("Calib.VGR") {}
    bool load();
    bool saveDefault() { return true; }
    std::map<std::string, EncPos3> map_pos3;
    void setPos3(std::string k, EncPos3 p) { map_pos3[k] = p; }
    void copyPos3X(std::string s, std::string d) {} 
    void copyPos3Y(std::string s, std::string d) {}
    void copyPos3Z(std::string s, std::string d) {}
};

class TxtVacuumGripperRobot : public TxtSimulationModel {
public:
    enum State_t {
        __NO_STATE,
        FSM_DECLARE_STATE_XE( FAULT ),
        FSM_DECLARE_STATE_XE( INIT ),
        FSM_DECLARE_STATE_XE( IDLE ),
        FSM_DECLARE_STATE_XE( FETCH_WP_VGR ),
        FSM_DECLARE_STATE_XE( VGR_WAIT_FETCHED ),
        FSM_DECLARE_STATE_XE( MOVE_VGR2MPO ),
        FSM_DECLARE_STATE_XE( START_PRODUCE ),
        FSM_DECLARE_STATE_XE( MOVE_PICKUP_WAIT ),
        FSM_DECLARE_STATE_XE( MOVE_PICKUP ),
        FSM_DECLARE_STATE_XE( START_DELIVERY ),
        FSM_DECLARE_STATE_XE( COLOR_DETECTION ),
        FSM_DECLARE_STATE_XE( WRONG_COLOR ),
        FSM_DECLARE_STATE_XE( NFC_RAW ),
        FSM_DECLARE_STATE_XE( NFC_PRODUCED ),
        FSM_DECLARE_STATE_XE( NFC_REJECTED ),
        FSM_DECLARE_STATE_XE( STORE_WP_VGR ),
        FSM_DECLARE_STATE_XE( STORE_WP ),
        FSM_DECLARE_STATE_XE( CALIB_HBW ),
        FSM_DECLARE_STATE_XE( CALIB_SLD ),
        FSM_DECLARE_STATE_XE( CALIB_DPS ),
        FSM_DECLARE_STATE_XE( CALIB_DPS_NEXT ),
        FSM_DECLARE_STATE_XE( CALIB_VGR ),
        FSM_DECLARE_STATE_XE( CALIB_VGR_NAV ),
        FSM_DECLARE_STATE_XE( CALIB_VGR_MOVE ),
    };

    TxtVacuumGripperRobot(TxtTransfer* pT, TxtMqttFactoryClient* mqttclient = 0);
    virtual ~TxtVacuumGripperRobot();

    void requestQuit() { reqQuit= true; }
    void requestOrder(TxtWPType_t type);
    void requestNfcRead() { reqNfcRead= true; }
    void requestNfcDelete() { reqNfcDelete= true; }
    void requestHBWstored(TxtWorkpiece* wp) { reqWP_HBW=wp; reqHBWstored=true; }
    void requestHBWfetched(TxtWorkpiece* wp) { reqWP_HBW=wp; reqHBWfetched=true; }
    
    // Sim Helpers
    void setDetectedColor(TxtWPType_t c);
    TxtNfcDevice* getNfcDevice() { return dps.getNfc(); }

    void run();

    TxtAxis1RefSwitch axisX, axisY, axisZ;

protected:
    State_t currentState;
    State_t newState;
    TxtVgrCalibPos_t calibPos;
    TxtWPType_t calibColor;
    EncPos3 lastPos3;

    void configInputs();
    void initDashboard();
    void move(const std::string pos3name, TxtVgrPosOrder_t order = VGRMOV_PTP);
    void move(EncPos3 p3, TxtVgrPosOrder_t order = VGRMOV_PTP) {}
    void move(uint16_t x, uint16_t y, uint16_t z, TxtVgrPosOrder_t order = VGRMOV_PTP);
    void moveRef();
    void stop();
    void setSpeed(int16_t s);
    EncPos3 getPos3() { return {0,0,0}; }

    // Logic Methods
    void grip() { vgripper.grip(); }
    void release() { vgripper.release(); }
    void moveDeliveryInAndGrip();
    void moveDeliveryOutAndRelease();
    void moveColorSensor(bool half = false);
    void moveRefYNFC();
    void moveNFC();
    void moveWrongRelease();
    void moveToHBW();
    void moveFromHBW1();
    void moveFromHBW2();
    void moveMPO();
    
    const char * toString(State_t state);
    void printEntryState(State_t state) { std::cout << "Enter: " << toString(state) << std::endl; }
    void printState(State_t state) {}

    void fsmStep();

    TxtVacuumGripper vgripper;
    TxtVacuumGripperRobotCalibData calibData;
    std::string target;
    TxtDeliveryPickupStation dps;
    TxtFactoryProcessStorage proStorage;

    bool reqQuit;
    bool reqOrder;
    TxtWorkpiece reqWP_order;
    bool reqNfcRead, reqNfcDelete;
    TxtOrderState ord_state;
    TxtJoysticksData joyData;
    bool reqJoyData;
    bool reqMPOstarted;
    TxtWorkpiece* reqWP_MPO;
    bool reqHBWstored, reqHBWfetched;
    bool reqHBWcalib_nav, reqHBWcalib_end, reqSLDcalib_end;
    TxtWorkpiece* reqWP_HBW;
    bool reqSLDsorted;
    TxtWorkpiece reqWP_SLD;
    int calibColorValues[3];

    TxtVacuumGripperRobotObserver* obs_vgr;
    TxtNfcDeviceObserver* obs_nfc;
    TxtDeliveryPickupStationObserver* obs_dps;
    TxtMqttFactoryClient* mqttclient;
    
    // Simulation state
    TxtWPType_t simDetectedColor;
};

class TxtVacuumGripperRobotObserver : public Observer {
public:
    TxtVacuumGripperRobotObserver(TxtVacuumGripperRobot* s, TxtMqttFactoryClient* m) : _subject(s), _mqttclient(m) { _subject->Attach(this); }
    virtual ~TxtVacuumGripperRobotObserver() { _subject->Detach(this); }
    void Update(SubjectObserver*) {}
private:
    TxtVacuumGripperRobot *_subject;
    TxtMqttFactoryClient* _mqttclient;
};

}
#endif