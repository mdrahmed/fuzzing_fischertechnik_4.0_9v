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
#define SPDLOG_LOGGER_TRACE(...) 
#define SPDLOG_LOGGER_DEBUG(logger, fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#define TIMEOUT_MS_PUBLISH 100

namespace ft {
    enum TxtWPType_t { WP_TYPE_NONE=0, WP_TYPE_WHITE, WP_TYPE_RED, WP_TYPE_BLUE };
    enum TxtWPState_t { WP_STATE_RAW=0, WP_STATE_PROCESSED, WP_STATE_REJECTED };
    enum TxtLEDSCode_t { LEDS_OFF=0, LEDS_READY=1, LEDS_BUSY=2, LEDS_ERROR=4, LEDS_WAIT_READY=5 };
    
    #define HBW_FETCHED 1
    #define HBW_CALIB_NAV 2
    #define HBW_CALIB_END 3
    #define HBW_EXIT 4

    struct TxtWorkpiece {
        std::string tag_uid;
        TxtWPType_t type;
        TxtWPState_t state;
        TxtWorkpiece(std::string uid="", TxtWPType_t t=WP_TYPE_NONE, TxtWPState_t s=WP_STATE_RAW)
            : tag_uid(uid), type(t), state(s) {}
    };

    struct EncPos2 { int x, y; EncPos2(int _x=0,int _y=0):x(_x),y(_y){} };
    struct TxtJoysticksData { int aX1=0, aY1=0, b1=0, aX2=0, aY2=0, b2=0; };
    typedef std::map<std::string, TxtWorkpiece*> Stock_map_t;

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
        virtual void publishStateHBW(TxtLEDSCode_t, std::string, int, int, std::string) {}
        virtual void publishHBW_Ack(int, void*, int) { std::cout << "[MQTT] HBW Ack Sent.\n"; }
        virtual void publishStock(Stock_map_t, int) { std::cout << "[MQTT] Stock Updated.\n"; }
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

    // --- Axis Mocks ---
    class TxtAxis {
    protected:
        int pos = 0;
        std::string name;
    public:
        TxtAxis(std::string n, TxtTransfer* t, int ch, int chS, int end) : name(n) {}
        void stop() { std::cout << "[" << name << "] Stopped.\n"; }
        void setSpeed(int s) {}
        int getPosAbs() { return pos; }
        bool moveAbs(int target) { 
            std::cout << "[" << name << "] Moving to " << target << "...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
            pos = target; 
            return true; 
        }
        std::thread moveAbsThread(int target) { return std::thread([this, target](){ moveAbs(target); }); }
        std::thread moveRefThread() { 
            return std::thread([this](){ 
                std::cout << "[" << name << "] Homing...\n"; 
                pos = 0; 
            }); 
        }
        void moveS1() {}
        void moveS2() {}
        void moveRel(int d) { pos += d; }
    };

    class TxtAxis1RefSwitch : public TxtAxis { using TxtAxis::TxtAxis; };
    class TxtAxisNSwitch : public TxtAxis { using TxtAxis::TxtAxis; };

    class TxtConveyorBeltLightBarriers {
    public:
        TxtConveyorBeltLightBarriers(TxtTransfer* t, int a, int b, int c) {}
        void moveIn() { std::cout << "[Conv] Moving In\n"; }
        void moveOut() { std::cout << "[Conv] Moving Out\n"; }
    };

    // --- Sound Mock ---
    struct Sound {
        void error() { std::cout << "[Sound] *BEEP* Error\n"; }
        void info1() { std::cout << "[Sound] *BEEP* Info1\n"; }
        void info2() { std::cout << "[Sound] *BEEP* Info2\n"; }
    };
    static Sound sound;
}
#endif