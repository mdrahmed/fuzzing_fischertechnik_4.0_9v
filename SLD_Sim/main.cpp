#include "TxtSortingLine.h"
#include <iostream>
#include <thread>
#include <string>

void printHelp() {
    std::cout << "Commands:\n"
              << "  white  -> Process White Workpiece\n"
              << "  red    -> Process Red Workpiece\n"
              << "  blue   -> Process Blue Workpiece\n"
              << "  quit   -> Exit Simulation\n"
              << "> ";
}

int main() {
    std::cout << "=== Sorting Line (SLD) Simulation ===\n";

    // Mocks
    ft::TxtTransfer transfer;
    ft::TxtMqttFactoryClient mqtt;

    // Instance
    ft::TxtSortingLine sld(&transfer, &mqtt);

    // Run Logic in Background Thread
    std::thread runner([&sld](){
        sld.run();
    });

    std::string input;
    while(true) {
        printHelp();
        std::cin >> input;

        if (input == "quit") {
            // Force exit logic
            exit(0); 
        } 
        else if (input == "white") {
            std::cout << "-> Inputting White Piece...\n";
            sld.setInputColor(ft::WP_TYPE_WHITE);
        }
        else if (input == "red") {
            std::cout << "-> Inputting Red Piece...\n";
            sld.setInputColor(ft::WP_TYPE_RED);
        }
        else if (input == "blue") {
            std::cout << "-> Inputting Blue Piece...\n";
            sld.setInputColor(ft::WP_TYPE_BLUE);
        }
        else {
            std::cout << "Unknown command.\n";
        }

        // Wait a bit to let the FSM process the idle state trigger
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    runner.join();
    return 0;
}