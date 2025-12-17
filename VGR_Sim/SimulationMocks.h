#ifndef SIMULATION_MOCKS_H
#define SIMULATION_MOCKS_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cassert>
#include <cmath>
#include <map>
#include <mutex>

// --- Macros ---
// Redirect SPDLog to std::cout
#define SPDLOG_LOGGER_TRACE(...) 
#define SPDLOG_LOGGER_DEBUG(logger, fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#define TIMEOUT_MS_PUBLISH 100

// --- Mock Enums & Constants ---
namespace ft {
    enum TxtWPType_t { WP_TYPE_NONE=0, WP_TYPE_WHITE, WP_TYPE_RED, WP_TYPE_BLUE };
    enum TxtWPState_t { WP_STATE_RAW=0, WP_STATE_PROCESSED, WP_STATE_REJECTED };
    enum TxtLEDSCode_t { LEDS_OFF=0, LEDS_READY=1, LEDS_BUSY=2, LEDS_ERROR=4, LEDS_WAIT_READY=5 };
    enum TxtHistoryIndex_t { DELIVERY_RAW_INDEX=0, INSPECTION_INDEX, WAREHOUSING_INDEX, OUTSOURCING_INDEX, PROCESSING_OVEN_INDEX, SORTING_INDEX, SHIPPING_INDEX, NUM_INDEX_MAX };
    typedef int TxtHistoryCode_t; 
    inline TxtHistoryCode_t toCode(TxtHistoryIndex_t i) { return (TxtHistoryCode_t)i; }

    #define VGR_HBW_FETCH_WP 1
    #define VGR_HBW_STORECONTAINER 2
    #define VGR_MPO_PRODUCE 3
    #define VGR_HBW_FETCHCONTAINER 4
    #define VGR_HBW_STORE_WP 5
    #define VGR_HBW_CALIB 6
    #define VGR_SLD_CALIB 7
    #define VGR_HBW_RESETSTORAGE 8
    #define VGR_EXIT 9

    struct TxtWorkpiece {
        std::string tag_uid;
        TxtWPType_t type;
        TxtWPState_t state;
        TxtWorkpiece(std::string uid="", TxtWPType_t t=WP_TYPE_NONE, TxtWPState_t s=WP_STATE_RAW)
            : tag_uid(uid), type(t), state(s) {}
        void printDebug() { std::cout << "WP: " << tag_uid << " Type: " << type << "\n"; }
    };

    struct EncPos3 { int x, y, z; EncPos3(int _x=0,int _y=0,int _z=0):x(_x),y(_y),z(_z){} };
    struct TxtJoysticksData { int aX1=0, aY1=0, b1=0, aX2=0, aY2=0, b2=0; };
    enum TxtOrderState_t { WAITING_FOR_ORDER, ORDERED, IN_PROCESS, SHIPPED };
    struct TxtOrderState { TxtWPType_t type; TxtOrderState_t state; };
    typedef std::map<TxtHistoryCode_t, int64_t> History_map_t;

    // --- Observer Pattern ---
    class SubjectObserver;
    class Observer {
    public:
        virtual ~Observer() {}
        virtual void Update(SubjectObserver* changedSubject) = 0;
    };

    class SubjectObserver {
        std::vector<Observer*> observers;
    public:
        void Attach(Observer* o) { observers.push_back(o); }
        void Detach(Observer* o) { /* simplified */ }
        void Notify() { for(auto o : observers) o->Update(this); }
    };

    // --- Hardware Mocks ---
    class TxtTransfer { 
    public: 
        struct {
            struct { int uni[8]; int cnt_in[4]; } ftX1in;
            struct { struct { int mode; int digital; } uni[8]; } ftX1config;
            struct { int config_id; } ftX1state;
            struct { int duty[8]; } ftX1out;
        } *pTArea;
        TxtTransfer() { pTArea = new std::remove_reference<decltype(*pTArea)>::type(); }
    };
    struct FISH_X1_TRANSFER {
        struct { int ComErr=0; int iostatus=0; } IFStatus;
        struct { int uni[8]={0}; int cnt_in[4]={0}; } ftX1in;
    };

    class TxtMqttFactoryClient {
    public:
        void publishStateVGR(TxtLEDSCode_t, std::string, int, int, std::string) {}
        void publishVGR_Do(int, void*, int) { std::cout << "[MQTT] VGR Do Command Sent.\n"; }
        void publishStateOrder(TxtOrderState, int) {}
        void publishNfcDS(TxtWorkpiece, History_map_t, int) {}
        void publishStateHBW(TxtLEDSCode_t, std::string, int, int, std::string) {}
        void publishStateMPO(TxtLEDSCode_t, std::string, int, int, std::string) {}
        void publishStateSLD(TxtLEDSCode_t, std::string, int, int, std::string) {}
        void publishStateDSI(TxtLEDSCode_t, std::string, int, int, std::string) {}
        void publishStateDSO(TxtLEDSCode_t, std::string, int, int, std::string) {}
    };

    enum TxtSimulationModel_status_t { SM_READY, SM_BUSY, SM_ERROR, SM_CALIB };

    class TxtSimulationModel : public SubjectObserver {
    protected:
        TxtSimulationModel_status_t status = SM_READY;
        bool m_stoprequested = false;
    public:
        TxtSimulationModel(TxtTransfer* t, TxtMqttFactoryClient* m) {}
        virtual ~TxtSimulationModel() {}
        void setActStatus(bool active, TxtSimulationModel_status_t s) { status = s; }
        void setStatus(TxtSimulationModel_status_t s) { status = s; }
        TxtSimulationModel_status_t getStatus() { return status; }
        bool isActive() { return status == SM_BUSY; }
        void SetTransferAreaCompleteCallback(bool (*func)(FISH_X1_TRANSFER*, int)) {}
        void stopThread() { m_stoprequested = true; } 
    };

    class TxtCalibData {
    public:
        TxtCalibData(std::string f) {}
        bool existCalibFilename() { return false; }
        virtual bool load() { return true; }
        virtual bool saveDefault() { return true; }
        virtual bool save() { return true; }
        bool valid = false;
        std::string filename = "";
    };

    class TxtAxis1RefSwitch {
    public:
        TxtAxis1RefSwitch(std::string, TxtTransfer*, int, int, int) {}
        void stop() {}
        void moveRef() { std::cout << "[Axis] Homing...\n"; }
        std::thread moveRefThread() { return std::thread([this]{ moveRef(); }); }
        void moveAbs(int) { std::cout << "[Axis] Moving Abs...\n"; }
        std::thread moveAbsThread(int) { return std::thread([this]{ moveAbs(0); }); }
        int getPosAbs() { return 0; }
        int getPosEnd() { return 1000; }
        void setSpeed(int) {}
        void moveRel(int) {}
    };

    // --- Sound Mock ---
    struct Sound {
        void error() { std::cout << "[Sound] *BEEP* Error\n"; }
        void info1() { std::cout << "[Sound] *BEEP* Info1\n"; }
        void info2() { std::cout << "[Sound] *BEEP* Info2\n"; }
        void warn() { std::cout << "[Sound] *BEEP* Warn\n"; }
    };
    static Sound sound;

    // --- Utils ---
    inline int64_t getnowtimestamp_s() { return 0; }
    inline void gettimestampstr(int64_t, char*) {}
}
#endif