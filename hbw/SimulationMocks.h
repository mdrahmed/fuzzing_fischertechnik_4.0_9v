#ifndef SIMULATION_MOCKS_H
#define SIMULATION_MOCKS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <cmath>
#include <mutex>

// --- Macros Replacements ---
#define SPDLOG_LOGGER_TRACE(...) 
#define SPDLOG_LOGGER_DEBUG(logger, fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#define TIMEOUT_MS_PUBLISH 100

// --- Enums & Structs ---
namespace ft {

enum TxtWPType_t { WP_TYPE_NONE = 0, WP_TYPE_WHITE, WP_TYPE_RED, WP_TYPE_BLUE };
enum TxtWPState_t { WP_STATE_RAW = 0, WP_STATE_PROCESSED, WP_STATE_REJECTED };
enum TxtLEDSCode_t { LEDS_OFF = 0, LEDS_READY = 1, LEDS_BUSY = 2, LEDS_ERROR = 4 };

struct TxtWorkpiece {
    std::string tag_uid;
    TxtWPType_t type;
    TxtWPState_t state;
    TxtWorkpiece(std::string uid="", TxtWPType_t t=WP_TYPE_NONE, TxtWPState_t s=WP_STATE_RAW)
        : tag_uid(uid), type(t), state(s) {}
};

struct EncPos2 { 
    int x; int y; 
    EncPos2(int _x=0, int _y=0) : x(_x), y(_y) {}
};

struct TxtJoysticksData {
    int aX1=0, aY1=0, b1=0;
    int aX2=0, aY2=0, b2=0;
};

// --- Observer Pattern Mock ---
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

// --- Hardware/Communication Mocks ---
class TxtTransfer { public: int* pTArea = nullptr; };

class TxtMqttFactoryClient {
public:
    void publishStateHBW(TxtLEDSCode_t, std::string, int, int, std::string) {}
    void publishHBW_Ack(int, void*, int) { std::cout << "[MQTT] Ack sent.\n"; }
    void publishStock(std::map<std::string, TxtWorkpiece*>, int) {}
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
    bool existCalibFilename() { return false; } // Force default
    virtual bool load() { return true; }
    virtual bool saveDefault() { return true; }
    virtual bool save() { return true; }
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
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate movement
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
    void error() { std::cout << "[Sound] Error Beep\n"; }
    void info1() { std::cout << "[Sound] Info1\n"; }
    void info2() { std::cout << "[Sound] Info2\n"; }
};
// FIX: Added 'static' here to prevent duplicate symbol error
static Sound sound;

} // namespace
#endif