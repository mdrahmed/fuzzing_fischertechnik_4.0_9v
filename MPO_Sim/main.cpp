#include "TxtMultiProcessingStation.h"
#include <iostream>
#include <thread>
#include <string>

void printHelp() {
    std::cout << "Commands:\n"
              << "  white  -> Produce White Workpiece\n"
              << "  red    -> Produce Red Workpiece\n"
              << "  blue   -> Produce Blue Workpiece\n"
              << "  quit   -> Exit Simulation\n"
              << "> ";
}

int main() {
    std::cout << "=== Multi-Processing Station (MPO) Simulation ===\n";

    // Mocks
    ft::TxtTransfer transfer;
    ft::TxtMqttFactoryClient mqtt;

    // Instance
    ft::TxtMultiProcessingStation mpo(&transfer, &mqtt);

    // Run Logic in Background Thread
    std::thread runner([&mpo](){
        mpo.run();
    });

    // Wait for init
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::string input;
    while(true) {
        printHelp();
        std::cin >> input;

        if (input == "quit") {
            exit(0); 
        } 
        else if (input == "white") {
            std::cout << "-> Requesting White...\n";
            mpo.triggerOven(); // Simulate placing item in oven
            ft::TxtWorkpiece wp("TAG_W", ft::WP_TYPE_WHITE, ft::WP_STATE_RAW);
            mpo.requestVGRproduce(&wp);
            // Simulate transport time then trigger end sensor
            std::this_thread::sleep_for(std::chrono::seconds(2));
            mpo.triggerEndBelt();
        }
        else if (input == "red") {
            std::cout << "-> Requesting Red...\n";
            mpo.triggerOven();
            ft::TxtWorkpiece wp("TAG_R", ft::WP_TYPE_RED, ft::WP_STATE_RAW);
            mpo.requestVGRproduce(&wp);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            mpo.triggerEndBelt();
        }
        else if (input == "blue") {
            std::cout << "-> Requesting Blue...\n";
            mpo.triggerOven();
            ft::TxtWorkpiece wp("TAG_B", ft::WP_TYPE_BLUE, ft::WP_STATE_RAW);
            mpo.requestVGRproduce(&wp);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            mpo.triggerEndBelt();
        }
        else {
            std::cout << "Unknown command.\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    runner.join();
    return 0;
}