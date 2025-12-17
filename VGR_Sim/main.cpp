#include "TxtVacuumGripperRobot.h"
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <vector>

// Simple storage simulation for visualization
ft::TxtWPType_t storage[3][3] = { {ft::WP_TYPE_NONE} };

void printStorage() {
    std::cout << "\n--- Storage ---\n";
    for(int y=2; y>=0; y--) {
        for(int x=0; x<3; x++) {
            char c = '0';
            if(storage[x][y] == ft::WP_TYPE_WHITE) c = 'W';
            else if(storage[x][y] == ft::WP_TYPE_RED) c = 'R';
            else if(storage[x][y] == ft::WP_TYPE_BLUE) c = 'B';
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
    std::cout << "---------------\n> ";
}

ft::TxtWPType_t parseColor(std::string s) {
    if(s == "white") return ft::WP_TYPE_WHITE;
    if(s == "red") return ft::WP_TYPE_RED;
    if(s == "blue") return ft::WP_TYPE_BLUE;
    return ft::WP_TYPE_NONE;
}

bool updateStorageStore(ft::TxtWPType_t type) {
    for(int x=0; x<3; x++) {
        for(int y=0; y<3; y++) {
            if(storage[x][y] == ft::WP_TYPE_NONE) {
                storage[x][y] = type;
                return true;
            }
        }
    }
    return false;
}

bool updateStorageFetch(ft::TxtWPType_t type) {
    for(int x=0; x<3; x++) {
        for(int y=2; y>=0; y--) {
            if(storage[x][y] == type) {
                storage[x][y] = ft::WP_TYPE_NONE;
                return true;
            }
        }
    }
    return false;
}

int main() {
    ft::TxtTransfer transfer;
    ft::TxtMqttFactoryClient mqtt;
    ft::TxtVacuumGripperRobot vgr(&transfer, &mqtt);

    std::thread vgrThread([&vgr](){ vgr.run(); });

    std::cout << "Example Input: store white, store red, fetch white\n";

    while(true) {
        printStorage();
        std::string line, cmd, colorStr;
        if (!std::getline(std::cin, line) || line == "quit") break;
        
        std::stringstream ss(line);
        ss >> cmd >> colorStr;
        
        ft::TxtWPType_t type = parseColor(colorStr);
        if (type == ft::WP_TYPE_NONE) {
            std::cout << "Invalid color.\n";
            continue;
        }

        if (cmd == "store") {
            if (updateStorageStore(type)) {
                // Simulate putting item at input
                vgr.setDetectedColor(type); 
                // Wait for FSM to process
                std::this_thread::sleep_for(std::chrono::seconds(3));
            } else {
                std::cout << "Storage Full!\n";
            }
        } else if (cmd == "fetch") {
            if (updateStorageFetch(type)) {
                vgr.requestOrder(type);
                std::this_thread::sleep_for(std::chrono::seconds(3));
            } else {
                std::cout << "Item not found in storage!\n";
            }
        } else {
            std::cout << "Unknown command.\n";
        }
    }

    vgr.stopThread();
    vgrThread.join();
    return 0;
}