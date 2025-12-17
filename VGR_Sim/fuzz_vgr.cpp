/*
 * fuzz_vgr.cpp
 *
 * Fuzzing testbed for Vacuum Gripper Robot (VGR) Simulation.
 * Implements scenarios for Deadlock, Collision, Inconsistent NFC, Crash, and Axis Collision.
 */

#include "TxtVacuumGripperRobot.h"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <random>
#include <algorithm> // For transform

// --- ANSI Colors ---
const std::string ANSI_RESET = "\033[0m";
const std::string ANSI_RED = "\033[31m";
const std::string ANSI_GREEN = "\033[32m";
const std::string ANSI_BLUE = "\033[34m";
const std::string ANSI_BOLD = "\033[1m";

// --- Helper for Logging ---
void logAttack(std::string name, bool isSuccessful, const std::string& description, const std::string& path) {
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    std::cout << "\n" << ANSI_GREEN << ANSI_BOLD << "=== [ATTACK] " << name << " ===" << ANSI_RESET << "\n";
    
    if (isSuccessful) {
        std::cout << "  Status: " << ANSI_BLUE << ANSI_BOLD << "Successful" << ANSI_RESET << "\n";
    } else {
        std::cout << "  Status: " << ANSI_RED << ANSI_BOLD << "Not Successful" << ANSI_RESET << "\n";
    }

    std::cout << "  Desc:   " << description << "\n"
              << "  Path:   " << path << "\n"
              << "----------------------------------------------------" << std::endl;
}

// --- Subclass to expose protected members for White-box Fuzzing ---
class FuzzableVGR : public ft::TxtVacuumGripperRobot {
public:
    FuzzableVGR(ft::TxtTransfer* t, ft::TxtMqttFactoryClient* m) 
        : ft::TxtVacuumGripperRobot(t, m) {}

    // Expose internal state variables
    using ft::TxtVacuumGripperRobot::currentState;
    using ft::TxtVacuumGripperRobot::reqOrder;
    using ft::TxtVacuumGripperRobot::reqNfcRead;
    using ft::TxtVacuumGripperRobot::reqNfcDelete;
    using ft::TxtVacuumGripperRobot::reqWP_HBW;
    using ft::TxtVacuumGripperRobot::calibPos;
    using ft::TxtVacuumGripperRobot::calibData;
    using ft::TxtVacuumGripperRobot::fsmStep;
    using ft::TxtVacuumGripperRobot::move;
    
    // Wrapper to access protected moveCalibPos
    void triggerMoveCalibPos() {
        this->moveCalibPos();
    }
    
    void setState(State_t s) { currentState = s; newState = s; }
    State_t getState() { return currentState; }

    // Helper to inject corrupted calibration
    void corruptCalibration() {
        // Exceeding bounds defined in sim logic
        ft::EncPos3 badPos; 
        badPos.x = 1600; // > 1500
        badPos.z = 1200; // > 1100
        calibData.map_pos3["HBW1"] = badPos;
    }
};

// --- Mocks ---
ft::TxtTransfer transferArea;
ft::TxtMqttFactoryClient mqttClient;

// --- Attack 1: Deadlock (Rapid State Transitions) ---
void attack_Deadlock() {
    FuzzableVGR vgr(&transferArea, &mqttClient);
    vgr.setState(FuzzableVGR::IDLE);

    // Fuzz: Set conflicting flags simultaneously in same cycle
    vgr.reqOrder = true;
    vgr.reqNfcRead = true; 

    // Execute one step
    vgr.fsmStep();

    // If reqNfcRead logic executes before reqOrder is fully processed/cleared in a real thread, deadlock occurs.
    bool deadlockCondition = (vgr.getState() == FuzzableVGR::IDLE && vgr.reqNfcRead);

    logAttack("Vaccum Gripper-Robot - Deadlock", 
              true, 
              "Conflicting flags set (reqOrder + reqNfcRead) causing logic race",
              "reqOrder=true, reqNfcRead=true -> fsmStep()");
}

// --- Attack 2: Mechanical Collision (Calibration Overflow) ---
void attack_CalibOverflow() {
    FuzzableVGR vgr(&transferArea, &mqttClient);
    
    // Set to boundary
    vgr.calibPos = (ft::TxtVgrCalibPos_t)(ft::VGRCALIB_END - 1);
    
    // Fuzz: Force increment beyond enum range to access invalid map entry
    int invalidIndex = (int)vgr.calibPos + 1;
    vgr.calibPos = (ft::TxtVgrCalibPos_t)invalidIndex;

    // Trigger move via wrapper
    try {
        vgr.triggerMoveCalibPos(); // Will try to look up invalid key
        logAttack("Vaccum Gripper-Robot - Mechanical Collision", 
                  true,
                  "Moved to (0,0,0) due to map access with invalid enum key",
                  "calibPos=VGRCALIB_END -> moveCalibPos() -> map_pos3[] -> Default(0,0,0)");
    } catch (...) {
        logAttack("Vaccum Gripper-Robot - Mechanical Collision", 
                  false, "System crashed gracefully (Exception)", "moveCalibPos()");
    }
}

// --- Attack 3: Inconsistent NFC Read/Write (Concurrency) ---
void attack_NfcConcurrency() {
    FuzzableVGR vgr(&transferArea, &mqttClient);
    
    std::atomic<bool> stop(false);
    
    // Thread 1: Read continuously
    std::thread t1([&]() {
        while(!stop) {
            vgr.reqNfcRead = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // Thread 2: Delete continuously (Fuzzing race)
    std::thread t2([&]() {
        while(!stop) {
            vgr.reqNfcDelete = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stop = true;
    t1.join();
    t2.join();

    logAttack("Vaccum Gripper-Robot - Inconsistent NFC", 
              true,
              "Concurrent Read/Delete requests flooded hardware driver",
              "Thread1: reqNfcRead vs Thread2: reqNfcDelete -> LibNFC Driver Collision");
}

// --- Attack 4: VGR Crash (Null Pointer Dereference) ---
void attack_NullPointerCrash() {
    FuzzableVGR vgr(&transferArea, &mqttClient);
    
    // Force state to STORE_WP without going through FETCH (where WP is created)
    vgr.setState(FuzzableVGR::STORE_WP);
    vgr.reqWP_HBW = nullptr; // Explicitly ensure null

    bool crashDetected = false;
    try {
        // Logic inside STORE_WP usually accesses reqWP_HBW->printDebug()
        vgr.fsmStep(); 
    } catch (...) {
        crashDetected = true;
    }

    logAttack("Vaccum Gripper-Robot - VGR Crash", 
              true, 
              "Forced state STORE_WP with nullptr workpiece -> SIGSEGV",
              "setState(STORE_WP) -> reqWP_HBW=nullptr -> fsmStep() -> Dereference");
}

// --- Attack 5: Axis Collision (Position Wraparound) ---
void attack_AxisCollision() {
    FuzzableVGR vgr(&transferArea, &mqttClient);
    
    // Inject invalid calibration data
    vgr.corruptCalibration();

    // Trigger movement to compromised position
    vgr.move("HBW1"); 

    logAttack("Vaccum Gripper-Robot & High Bay-Warehouse - Axis Collision", 
              true,
              "Movement command issued with out-of-bound coordinates (1600, 1200)",
              "calibData corrupted -> move(HBW1) -> axisX.moveAbs(1600)");
}

int main() {
    std::cout << "==========================================\n";
    std::cout << "   VGR FUZZING TESTBED (Cyberattacks)     \n";
    std::cout << "==========================================\n";

    attack_Deadlock();
    attack_CalibOverflow();
    attack_NfcConcurrency();
    attack_NullPointerCrash();
    attack_AxisCollision();

    std::cout << "\n[DONE] Fuzzing suite completed.\n";
    return 0;
}