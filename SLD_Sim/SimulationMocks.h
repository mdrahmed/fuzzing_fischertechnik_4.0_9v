#ifndef SIMULATION_MOCKS_H
#define SIMULATION_MOCKS_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cassert>
#include <cmath>

// --- Macros ---
#define SPDLOG_LOGGER_TRACE(...) 
#define SPDLOG_LOGGER_DEBUG(logger, fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#define TIMEOUT_MS_PUBLISH 100

// --- Mock Enums & Constants ---
namespace ft {
    enum TxtWPType_t { WP_TYPE_NONE=0, WP_TYPE_WHITE, WP_TYPE_RED, WP_TYPE_BLUE };
    enum TxtLEDSCode_t { LEDS_OFF=0, LEDS_READY=1, LEDS_BUSY=2, LEDS_ERROR=4 };
    
    // Ack Codes
    #define SLD_STARTED 1
    #define SLD_SORTED 2
    #define SLD_CALIB_END 3
    #define SLD_EXIT 4

    struct TxtJoysticksData { int b2=0; }; // Minimal mock

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
    class TxtTransfer { public: int* pTArea = nullptr; };

    class TxtMqttFactoryClient {
    public:
        void publishStateSLD(TxtLEDSCode_t, std::string, int, int, std::string) {}
        void publishSLD_Ack(int, TxtWPType_t, int, int) { std::cout << "[MQTT] Ack Sent.\n"; }
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
        void SetTransferAreaCompleteCallback(bool (*func)(void*, int)) {} 
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
    class TxtConveyorBelt {
    public:
        TxtConveyorBelt(TxtTransfer* t, int ch) {}
        void moveRight() { std::cout << "[Belt] Moving Right ->\n"; }
        void stop() { std::cout << "[Belt] Stopped.\n"; }
    };

    struct Sound {
        void error() { std::cout << "[Sound] *BEEP* Error\n"; }
        void info1() { std::cout << "[Sound] *BEEP* Info\n"; }
    };
    
    // FIX: 'static' prevents duplicate symbols during linking
    static Sound sound;
}

// Mock Global for sorting line callback
struct FISH_X1_TRANSFER {
    struct { int ComErr=0; int iostatus=0; } IFStatus;
    struct { int cnt_in[4]={0}; } ftX1in;
};
#endif