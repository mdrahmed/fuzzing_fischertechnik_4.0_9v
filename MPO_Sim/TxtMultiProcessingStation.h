#ifndef TxtMultiProcessingStation_H_
#define TxtMultiProcessingStation_H_

#include "SimulationMocks.h"

// FSM Helper
#define FSM_DECLARE_STATE_XE( stateName, attr... ) stateName

namespace ft {

class TxtMultiProcessingStationObserver;

class TxtMultiProcessingStationCalibData : public ft::TxtCalibData {
public:
    TxtMultiProcessingStationCalibData() : TxtCalibData("Data/Calib.MPO.json") {};
    bool load();
    bool saveDefault();
    bool save();
};

class TxtMultiProcessingStation : public ft::TxtSimulationModel {
public:
    enum State_t {
        __NO_STATE,
        FSM_DECLARE_STATE_XE( FAULT ),
        FSM_DECLARE_STATE_XE( INIT ),
        FSM_DECLARE_STATE_XE( IDLE ),
        FSM_DECLARE_STATE_XE( BURN ),
        FSM_DECLARE_STATE_XE( VGR_TRANSPORT ),
        FSM_DECLARE_STATE_XE( TABLE_SAW ),
        FSM_DECLARE_STATE_XE( TABLE_BELT ),
        FSM_DECLARE_STATE_XE( EJECT ),
        FSM_DECLARE_STATE_XE( TRANSPORT )
    };

    const char * toString(State_t state);
    void printEntryState(State_t state) { std::cout << "Enter: " << toString(state) << std::endl; }
    void printState(State_t state) {}

    TxtMultiProcessingStation(TxtTransfer* pT, ft::TxtMqttFactoryClient* mqttclient = 0);
    virtual ~TxtMultiProcessingStation();

    void requestQuit() { reqQuit= true; }
    void requestVGRproduce(TxtWorkpiece* wp) { reqVGRwp = wp; reqVGRproduce= true; }
    void requestSLDstarted() { reqSLDstarted= true; }

    bool isEndConveyorBeltTriggered();
    void setSawOff();
    void setSawLeft();
    void setSawRight();
    void setValveEjection(bool on);
    void setCompressor(bool on);
    bool isOvenTriggered();
    void setValveVacuum(bool on);
    void setValveLowering(bool on);
    void setValveOvenDoor(bool on);
    void setLightOven(bool on);

    TxtAxisNSwitch axisGripper;
    TxtAxisNSwitch axisOvenInOut;
    TxtAxisNSwitch axisRotTable;
    
    // Sim Helpers
    void triggerOven() { simOvenTriggered = true; }
    void triggerEndBelt() { simEndBeltTriggered = true; }
    void run();

protected:
    State_t currentState;
    State_t newState;
    void configInputs() {}
    void fsmStep();

    uint8_t chMsaw;
    TxtVacuumGripper vgripper;
    TxtConveyorBelt convBelt;
    TxtMultiProcessingStationCalibData calibData;

    bool reqQuit;
    TxtWorkpiece* reqVGRwp;
    bool reqVGRproduce;
    bool reqSLDstarted;

    TxtMultiProcessingStationObserver* obs_mpo;
    TxtMqttFactoryClient* mqttclient;
    
    // Sim State
    bool simOvenTriggered = false;
    bool simEndBeltTriggered = false;
    TxtTransfer* pT;
};

class TxtMultiProcessingStationObserver : public ft::Observer {
public:
    TxtMultiProcessingStationObserver(ft::TxtMultiProcessingStation* s, ft::TxtMqttFactoryClient* m)
        : _subject(s), _mqttclient(m) { _subject->Attach(this); }
    virtual ~TxtMultiProcessingStationObserver() { _subject->Detach(this); }
    void Update(ft::SubjectObserver* o) {}
private:
    ft::TxtMultiProcessingStation *_subject;
    ft::TxtMqttFactoryClient* _mqttclient;
};

} 
#endif