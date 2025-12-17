#ifndef TXTSORTINGLINE_H_
#define TXTSORTINGLINE_H_

#include "SimulationMocks.h"

// FSM Helper Macros
#define FSM_DECLARE_STATE_XE( stateName, attr... ) stateName

namespace ft {

class TxtSortingLineObserver;
extern uint16_t u16Counter;

class TxtSortingLineCalibData : public ft::TxtCalibData {
public:
    TxtSortingLineCalibData() : TxtCalibData("Data/Calib.SLD.json") {};
    bool load();
    bool saveDefault();
    bool save();
    int color_th[2];
    int count_white;
    int count_red;
    int count_blue;
};

class TxtSortingLine : public ft::TxtSimulationModel {
public:
    const uint16_t COUNT_WRONG = 29;

    enum State_t {
        __NO_STATE,
        FSM_DECLARE_STATE_XE( FAULT ),
        FSM_DECLARE_STATE_XE( INIT ),
        FSM_DECLARE_STATE_XE( IDLE ),
        FSM_DECLARE_STATE_XE( START ),
        FSM_DECLARE_STATE_XE( COLOR_DETECTION ),
        FSM_DECLARE_STATE_XE( START_COUNT ),
        FSM_DECLARE_STATE_XE( CHECK_COUNT ),
        FSM_DECLARE_STATE_XE( EJECTION_WHITE ),
        FSM_DECLARE_STATE_XE( EJECTION_RED ),
        FSM_DECLARE_STATE_XE( EJECTION_BLUE ),
        FSM_DECLARE_STATE_XE( SORTED ),
        FSM_DECLARE_STATE_XE( CALIB_SLD ),
        FSM_DECLARE_STATE_XE( CALIB_SLD_DETECTION ),
        FSM_DECLARE_STATE_XE( CALIB_SLD_NEXT ),
    };

    const char * toString(State_t state);
    void printEntryState(State_t state) { std::cout << "Enter: " << toString(state) << std::endl; }
    void printState(State_t state) {}

    TxtSortingLine(TxtTransfer* pT, ft::TxtMqttFactoryClient* mqttclient = 0);
    virtual ~TxtSortingLine();

    // Input simulation methods
    void setInputColor(TxtWPType_t color); // Helper for simulation
    void setJoyButton(bool b2) { joyData.b2 = b2; }

    void requestQuit() { reqQuit= true; }
    void requestVGRstart() { reqVGRstart= true; }
    void requestVGRcalib() { reqVGRcalib= true; }

    // Logic Methods
    bool isColorSensorTriggered();
    bool isEjectionTriggered();
    int readColorValue();
    ft::TxtWPType_t getLastColor();
    ft::TxtWPType_t getDetectedColor();

    void ejectWhite();
    void ejectRed();
    void ejectBlue();
    void setCompressor(bool on);
    void run();

    TxtConveyorBelt convBelt;
    TxtSortingLineCalibData calibData;

protected:
    State_t currentState;
    State_t newState;
    void configInputs() {} // Mocked
    void fsmStep();

    uint8_t chEW, chER, chEB, chComp;
    int lastColorValue;
    int detectedColorValue;
    TxtWPType_t calibColor;
    int calibColorValues[3];

    bool reqQuit;
    bool reqMPOproduced;
    bool reqVGRstart;
    bool reqVGRcalib;
    TxtJoysticksData joyData;
    bool reqJoyData;

    TxtSortingLineObserver* obs_sld;
    TxtMqttFactoryClient* mqttclient;

    // Simulation helpers
    TxtWPType_t simInputColor;
    int simEjectionCounter; 
};

class TxtSortingLineObserver : public ft::Observer {
public:
    TxtSortingLineObserver(ft::TxtSortingLine* s, ft::TxtMqttFactoryClient* m)
        : _subject(s), _mqttclient(m) { _subject->Attach(this); }
    virtual ~TxtSortingLineObserver() { _subject->Detach(this); }
    void Update(ft::SubjectObserver* o) { /* State update logic */ }
private:
    ft::TxtSortingLine *_subject;
    ft::TxtMqttFactoryClient* _mqttclient;
};

} 
#endif