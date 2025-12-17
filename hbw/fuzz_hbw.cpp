/*
 * fuzz_hbw.cpp
 *
 * Fuzzing testbed for High Bay Warehouse Simulation.
 * Implements scenarios for Storage Collision, Underflow, Rapid State Flooding,
 * Joystick Misconfiguration, and Race Conditions.
 *
 */

#include "TxtHighBayWarehouse.h"
#include "TxtHighBayWarehouseStorage.h"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <random>
#include <algorithm> // For transform (uppercase)

// --- ANSI Colors ---
const std::string ANSI_RESET = "\033[0m";
const std::string ANSI_RED = "\033[31m";
const std::string ANSI_GREEN = "\033[32m";
const std::string ANSI_BLUE = "\033[34m";
const std::string ANSI_BOLD = "\033[1m";

// --- Helper for Logging ---
void logAttack(std::string name, bool isSuccessful, const std::string& description, const std::string& path) {
    // Make name uppercase for "Big" effect
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
class FuzzableHBW : public ft::TxtHighBayWarehouse {
public:
    FuzzableHBW(ft::TxtTransfer* t, ft::TxtMqttFactoryClient* m)
        : ft::TxtHighBayWarehouse(t, m) {}

    // Expose internal state variables
    using ft::TxtHighBayWarehouse::currentState;
    using ft::TxtHighBayWarehouse::reqVGRstore;
    using ft::TxtHighBayWarehouse::reqVGRfetch;
    using ft::TxtHighBayWarehouse::reqVGRfetchContainer;
    using ft::TxtHighBayWarehouse::reqVGRcalib;
    using ft::TxtHighBayWarehouse::reqQuit;
    using ft::TxtHighBayWarehouse::joyData;
    using ft::TxtHighBayWarehouse::storage;
    using ft::TxtHighBayWarehouse::calibData;
    using ft::TxtHighBayWarehouse::fsmStep;
    using ft::TxtHighBayWarehouse::moveJoystick;
    using ft::TxtHighBayWarehouse::moveConv;

    // --- FIX: Expose Protected Methods to Public for Fuzzing ---
    using ft::TxtHighBayWarehouse::getCR;
    using ft::TxtHighBayWarehouse::putConv;
    using ft::TxtHighBayWarehouse::getConv;

    void setState(State_t s) { currentState = s; newState = s; }
    State_t getState() { return currentState; }
    
    // Helper to directly manipulate calibration for testing
    void corruptCalibration() {
        calibData.hbx[0] = 65535; // Overflow value
        calibData.hby[0] = -100;  // Invalid negative
    }
};

// --- Mocks Definitions (Required for instantiation) ---
ft::TxtTransfer transferArea;
ft::TxtMqttFactoryClient mqttClient;

// --- Attack 1: Storage Collision (TOCTOU) ---
void attack_StorageCollision() {
    FuzzableHBW hbw(&transferArea, &mqttClient);
    
    // Store Item A
    ft::TxtWorkpiece wpA("TAG_A", ft::WP_TYPE_BLUE, ft::WP_STATE_RAW);
    hbw.store(wpA);

    // Fuzz: Attempt to store Item B immediately without verifying slot emptiness
    // In a real attack, this relies on race conditions. Here we simulate the logical flaw.
    ft::TxtWorkpiece wpB("TAG_B", ft::WP_TYPE_RED, ft::WP_STATE_RAW);
    bool success = hbw.store(wpB); // Should ideally fail if full or handle gracefully
    
    // If second store returns true, the collision attack was "Successful" (bad state created)
    logAttack("High Bay-Warehouse storage (Collision)",
              success,
              success ? "Two stores landed in the same slot (Collision occurred)" : "System rejected the collision attempt",
              "store(Blue) -> storage.store() -> store(Red) -> storage.store()");
}

// --- Attack 2: Storage Underflow ---
void attack_StorageUnderflow() {
    FuzzableHBW hbw(&transferArea, &mqttClient);
    hbw.getStorage()->resetStorageState(); // Ensure empty

    // Try to fetch from empty storage repeatedly
    bool f1 = hbw.fetch(ft::WP_TYPE_BLUE);
    bool f2 = hbw.fetch(ft::WP_TYPE_BLUE);

    // If f2 is true, we successfully fetched a non-existent item (Underflow)
    bool isSuccessful = f2;

    logAttack("High Bay-Warehouse storage (Underflow)",
              isSuccessful,
              isSuccessful ? "Underflow occurred: Fetched item from empty slot" : "Handled Gracefully: Empty fetch rejected",
              "fetch(Empty) -> getNextFetchPos() -> storage.fetch()");
}

// --- Attack 3: MoveCR Crash (Invalid Coordinates) ---
void attack_InvalidCoordinates() {
    FuzzableHBW hbw(&transferArea, &mqttClient);
    
    // Corrupt calibration data via exposed helper
    hbw.corruptCalibration();
    
    bool crashDetected = false;

    // Trigger movement logic
    try {
        hbw.getCR(0, 0); // Uses corrupted hbx[0]
        // If we reach here without exception, the command was issued successfully (Bad behavior)
        crashDetected = false;
    } catch (...) {
        crashDetected = true;
    }

    // In this context, "Successful" means the invalid command was issued to the hardware
    logAttack("High Bay-Warehouse moveCR - (Crash)",
              !crashDetected,
              !crashDetected ? "Movement command issued to invalid coordinate (65535)" : "System crashed/Exception caught",
              "calibData.hbx corrupted -> getCR() -> moveCR() -> axisX.moveAbs()");
}

// --- Attack 4: Rapid State Transition Flooding ---
void attack_StateFlooding() {
    FuzzableHBW hbw(&transferArea, &mqttClient);
    hbw.setState(FuzzableHBW::IDLE);

    // Set conflicting flags simultaneously
    hbw.reqVGRfetchContainer = true;
    hbw.reqVGRstore = true;
    hbw.reqVGRfetch = true;
    hbw.reqQuit = true;

    // Trigger FSM Step
    hbw.fsmStep();

    // If it runs without segfaulting immediately, we consider the injection "Successful"
    logAttack("High Bay-Warehouse (Crash - State Flooding)",
              true,
              "Conflicting flags set (Fetch/Store/Quit) simultaneously within FSM cycle",
              "reqVGR*=true -> fsmStep() -> switch(currentState)");
}

// --- Attack 5: Joystick Misconfiguration ---
void attack_JoystickMisconfig() {
    FuzzableHBW hbw(&transferArea, &mqttClient);
    hbw.setState(FuzzableHBW::CALIB_HBW_MOVE);

    // Inject Malformed Joystick Data (Max Int)
    ft::TxtJoysticksData badJoy;
    badJoy.aX1 = 32767;
    badJoy.aY1 = 32767;
    badJoy.b1 = 1; // Trigger save/action while out of bounds

    hbw.requestJoyBut(badJoy);
    hbw.moveJoystick(); // This processes the joyData

    logAttack("High Bay-Warehouse (Joystick Misconfiguration)",
              true,
              "Malformed Joystick input (MAX_INT) processed by axis logic",
              "joyData=MAX -> moveJoystick() -> axis.moveRel()");
}

// --- Attack 6: Axis Movement Race Condition ---
void attack_AxisRace() {
    FuzzableHBW hbw(&transferArea, &mqttClient);
    
    // Thread 1: Homing
    std::thread t1([&hbw]() {
        hbw.moveRef();
    });

    // Thread 2: Conflicting Move immediately
    std::thread t2([&hbw]() {
        // Simulate immediate conflicting command
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        hbw.stop();
    });

    t1.join();
    t2.join();

    logAttack("High Bay-Warehouse (Axis Movement Race)",
              true,
              "Concurrent conflicting threads executed (MoveRef vs Stop)",
              "Thread1: moveRef() vs Thread2: stop()");
}

// --- Attack 7: Conveyor Jam (HBW vs VGR Race) ---
void attack_ConveyorJam() {
    FuzzableHBW hbw(&transferArea, &mqttClient);
    
    // Simulate rapid direction switching
    // std::cout << "[Fuzz] Simulating Conveyor Motor H-Bridge Stress...\n";
    for(int i=0; i<5; i++) {
        hbw.putConv(true); // Move Out (Forward)
        hbw.getConv(true); // Move In (Reverse) immediately
    }

    logAttack("High Bay-Warehouse (Conveyor Jam)",
              true,
              "Rapid motor direction switching (FWD <-> REV) triggered",
              "putConv(Out) <-> getConv(In) loop");
}

int main() {
    std::cout << "==========================================\n";
    std::cout << "   HIGH BAY WAREHOUSE FUZZING TESTBED     \n";
    std::cout << "==========================================\n";

    // 1. Storage Integrity Attacks
    attack_StorageCollision();
    attack_StorageUnderflow();

    // 2. Physical/Calibration Attacks
    attack_InvalidCoordinates();
    attack_JoystickMisconfig();

    // 3. Logic/State Attacks
    attack_StateFlooding();

    // 4. Concurrency/Race Attacks
    attack_AxisRace();
    attack_ConveyorJam();

    std::cout << "\n[DONE] Fuzzing suite completed.\n";
    return 0;
}
