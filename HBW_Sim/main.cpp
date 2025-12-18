#include "TxtHighBayWarehouse.h"
#include <iostream>
#include <thread>
#include <string>
#include <sstream>

ft::TxtWPType_t parseColor(std::string s) {
    if(s == "white") return ft::WP_TYPE_WHITE;
    if(s == "red") return ft::WP_TYPE_RED;
    if(s == "blue") return ft::WP_TYPE_BLUE;
    return ft::WP_TYPE_NONE;
}

int main() {
    std::cout << "=== HBW Simulation ===\n";

    ft::TxtTransfer transfer;
    ft::TxtMqttFactoryClient mqtt;
    ft::TxtHighBayWarehouse hbw(&transfer, &mqtt);

    // Run HBW Logic in thread
    std::thread runner([&hbw](){ hbw.run(); });

    // Wait for init
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Example Input: store white, fetch red\n";

    while(true) {
        hbw.printStorage();
        std::cout << "Command (store/fetch/quit) [color]: ";
        
        std::string line, cmd, colorStr;
        if (!std::getline(std::cin, line) || line == "quit") break;
        
        std::stringstream ss(line);
        ss >> cmd >> colorStr;
        
        ft::TxtWPType_t type = parseColor(colorStr);

        if (cmd == "store") {
            if (type == ft::WP_TYPE_NONE) { std::cout << "Invalid color\n"; continue; }
            auto wp = new ft::TxtWorkpiece("TAG_SIM", type, ft::WP_STATE_RAW);
            hbw.requestVGRstore(wp);
            // Wait for operation to complete visually in sim
            std::this_thread::sleep_for(std::chrono::seconds(3));
        } 
        else if (cmd == "fetch") {
            if (type == ft::WP_TYPE_NONE) { std::cout << "Invalid color\n"; continue; }
            auto wp = new ft::TxtWorkpiece("TAG_SIM", type, ft::WP_STATE_RAW);
            hbw.requestVGRfetch(wp);
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        else {
            std::cout << "Unknown command.\n";
        }
    }

    hbw.stopThread();
    runner.join();
    return 0;
}