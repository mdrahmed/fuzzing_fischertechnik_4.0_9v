#ifndef SIMULATION_MOCKS_H
#define SIMULATION_MOCKS_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cassert>
#include <cmath>
#include <mutex>

// --- Macros ---
#define SPDLOG_LOGGER_TRACE(...) 
#define SPDLOG_LOGGER_DEBUG(logger, fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#define TIMEOUT_MS_PUBLISH 100

// --- Mock Enums & Constants ---
namespace ft {
    enum TxtWPType_t { WP_TYPE_NONE=0, WP_TYPE_WHITE, WP_TYPE_RED, WP_TYPE_BLUE };
    enum TxtLEDSCode_t { LEDS_OFF=0, LEDS_READY=1, LEDS_BUSY=2, LEDS_ERROR=4 };
    enum TxtWPState_t { WP_STATE_RAW = 0, WP_STATE_PROCESSED, WP_STATE_REJECTED };

    // Ack Codes
    #define MPO_STARTED 1
    #define MPO_PRODUCED 2
    #define MPO_EXIT 3

    struct TxtWorkpiece {
        std::string tag_uid;
        TxtWPType_t type;
        TxtWPState_t state;
        TxtWorkpiece(std::string uid="", TxtWPType_t t=WP_TYPE_NONE, TxtWPState_t s=WP_STATE_RAW)
            : tag_uid(uid), type(t), state(s) {}
    };

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
            struct { int uni[8]; } ftX1in;
            struct { struct { int mode; int digital; } uni[8]; } ftX1config;
            struct { int config_id; } ftX1state;
            struct { int duty[8]; } ftX1out;
        } *pTArea;
        
        TxtTransfer() {
            // Allocate dummy memory to prevent segfaults on pTArea access
            pTArea = new std::remove_reference<decltype(*pTArea)>::type();
            // Simulate extension area (pTArea+1)
            // In a real mock we might need a dynamic array, but here we just need pointer arithmetic to work validly for +1
            // A simpler approach for simulation is to ignore the +1 logic or allocate array of 2
        }
    };

    class TxtMqttFactoryClient {
    public:
        void publishStateMPO(TxtLEDSCode_t, std::string, int, int, std::string) {}
        void publishMPO_Ack(int, int) { std::cout << "[MQTT] Ack Sent.\n"; }
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
    };

    class TxtCalibData {
    public:
        TxtCalibData(std::string f) {}
        bool existCalibFilename() { return false; }
        virtual bool load() { return true; }
        virtual bool saveDefault() { return true; }
        virtual bool save() { return true; }
    };

    // --- Components ---
    class TxtVacuumGripper {
    public:
        TxtVacuumGripper(TxtTransfer* t, int, int) {}
    };

    class TxtConveyorBelt {
    public:
        TxtConveyorBelt(TxtTransfer* t, int ch) {}
        void moveRight() { std::cout << "[Belt] Moving Right ->\n"; }
        void stop() { std::cout << "[Belt] Stopped.\n"; }
    };

    class TxtAxisNSwitch {
    public:
        std::string name;
        TxtAxisNSwitch(std::string n, TxtTransfer* t, int, int, int) : name(n) {}
        TxtAxisNSwitch(std::string n, TxtTransfer* t, int, int, int, int) : name(n) {}
        
        void moveS1() { std::cout << "[" << name << "] Move S1\n"; }
        void moveS2() { std::cout << "[" << name << "] Move S2\n"; }
        void moveS3() { std::cout << "[" << name << "] Move S3\n"; }
        
        std::thread moveS1Thread() { return std::thread([this]{ moveS1(); }); }
        std::thread moveS2Thread() { return std::thread([this]{ moveS2(); }); }
    };

    struct Sound {
        void error() { std::cout << "[Sound] *BEEP* Error\n"; }
        void info1() { std::cout << "[Sound] *BEEP* Info\n"; }
    };
    static Sound sound;
}
#endif