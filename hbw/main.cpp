#include "TxtHighBayWarehouse.h"
#include "SimulationMocks.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "=== Starting HBW Simulation ===\n";

    // Mocks
    ft::TxtTransfer transfer;
    ft::TxtMqttFactoryClient mqtt;

    // Instance
    ft::TxtHighBayWarehouse hbw(&transfer, &mqtt);

    // Run Logic in Thread
    std::thread runner([&hbw](){
        hbw.run();
    });

    // --- Fuzzing / Interaction Simulation ---
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for INIT -> IDLE

    std::cout << "\n>>> TEST: Store RED Workpiece\n";
    ft::TxtWorkpiece wp1("UID_1", ft::WP_TYPE_RED, ft::WP_STATE_RAW);
    hbw.requestVGRstore(&wp1);
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "\n>>> TEST: Fetch RED Workpiece\n";
    hbw.requestVGRfetch(&wp1);
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Cleanup
    std::cout << "\n>>> Stopping...\n";
    hbw.requestQuit(); // Stop FSM if in FAULT
    
    // Detach and exit (real code would use stop flag properly)
    runner.detach();

    return 0;
}