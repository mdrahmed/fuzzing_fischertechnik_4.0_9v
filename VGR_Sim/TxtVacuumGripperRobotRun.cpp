#include "TxtVacuumGripperRobot.h"

#define FSM_INIT_FSM( startState, attr... ) currentState = startState; newState = startState;
#define FSM_TRANSITION( _newState, attr... ) do { \
    std::cout << "FSM: " << toString(currentState) << " -> " << toString(_newState) << "\n"; \
    newState = _newState; } while(0)

namespace ft {

void TxtVacuumGripperRobot::fsmStep()
{
    if( newState != currentState ) {
        currentState = newState;
    }

    switch( currentState )
    {
    case INIT:
        std::cout << "\nINIT\n";
        moveRef();
        FSM_TRANSITION( IDLE );
        break;

    case IDLE:
        // std::cout << "\n--- IDLE ---\n";
        // 1. Process "Store" Command (Simulating receiving item at input)
        if (simDetectedColor != WP_TYPE_NONE && !reqOrder) {
            FSM_TRANSITION( START_DELIVERY );
        }
        // 2. Process "Fetch" Command (Order)
        else if (reqOrder) {
            reqOrder = false;
            FSM_TRANSITION( FETCH_WP_VGR );
        }
        break;

    case START_DELIVERY:
        std::cout << "\nSTART_DELIVERY\n";
        moveDeliveryInAndGrip();
        moveColorSensor();
        // Sim: Color detection result
        if (simDetectedColor != WP_TYPE_NONE) {
            // Assume valid for storage
            reqWP_HBW = new TxtWorkpiece("TAG_" + std::to_string(rand()), simDetectedColor, WP_STATE_RAW);
            FSM_TRANSITION( STORE_WP_VGR );
        } else {
            moveWrongRelease();
            FSM_TRANSITION( IDLE );
        }
        break;

    case STORE_WP_VGR:
        std::cout << "\nSTORE_WP_VGR\n";
        moveToHBW();
        reqHBWfetched = true;
        FSM_TRANSITION( STORE_WP );
        break;

    case STORE_WP:
        std::cout << "\nSTORE_WP\n";
        if (reqHBWfetched) {
            release(); // Place item in HBW
            moveRef();
            reqHBWfetched = false;
            simDetectedColor = WP_TYPE_NONE; // Reset input
            FSM_TRANSITION( IDLE );
        }
        break;

    case FETCH_WP_VGR:
        std::cout << "\nFETCH_WP_VGR\n";
        moveFromHBW1();
        reqHBWfetched = true;
        FSM_TRANSITION( VGR_WAIT_FETCHED );
        break;

    case VGR_WAIT_FETCHED:
        std::cout << "\nVGR_WAIT_FETCHED\n";
        if (reqHBWfetched) {
            moveFromHBW2(); // Pick up from HBW
            reqHBWfetched = false;
            moveMPO();
            moveRef();
            FSM_TRANSITION( IDLE );
        }
        break;

    default: break;
    }
}

void TxtVacuumGripperRobot::moveCalibPos() {
    std::cout << "[VGR] moveCalibPos index: " << (int)calibPos << "\n";
    
    std::string targetName = "";
    // Simulate mapping Enum to String Key
    switch (calibPos) {
        case VGRCALIB_DSI: targetName = "DIN"; break;
        case VGRCALIB_DCS: targetName = "DCS"; break;
        case VGRCALIB_NFC: targetName = "DNFC"; break;
        case VGRCALIB_WDC: targetName = "WDC"; break;
        case VGRCALIB_DSO: targetName = "DOUT"; break;
        case VGRCALIB_HBW: targetName = "HBW"; break;
        case VGRCALIB_MPO: targetName = "MPO"; break;
        case VGRCALIB_SL1: targetName = "SSD1"; break;
        case VGRCALIB_SL2: targetName = "SSD2"; break;
        case VGRCALIB_SL3: targetName = "SSD3"; break;
        default:
            // Vulnerability Simulation:
            // If index is out of bounds (e.g. VGRCALIB_END), map creates a default entry (0,0,0).
            targetName = "INVALID_POS_" + std::to_string((int)calibPos);
            break;
    }
    
    // Access map. If key doesn't exist, std::map creates it with default constructor (0,0,0)
    // This simulates the "Mechanical Collision" vulnerability logic.
    EncPos3 targetPos = calibData.map_pos3[targetName];
    
    std::cout << "[VGR] Calib Target '" << targetName << "': ("
              << targetPos.x << ", " << targetPos.y << ", " << targetPos.z << ")\n";

    // Physically move (simulated)
    move(targetPos.x, targetPos.y, targetPos.z);
    
    // Detect if we moved to origin (Crash condition)
    if (targetPos.x == 0 && targetPos.y == 0 && targetPos.z == 0) {
        std::cout << "[VGR] CRITICAL: Moved to (0,0,0) - Mechanical Collision Detected!\n";
    }
}

void TxtVacuumGripperRobot::run() {
    obs_vgr = new TxtVacuumGripperRobotObserver(this, mqttclient);
    
    FSM_INIT_FSM( INIT );
    while (!m_stoprequested) {
        fsmStep();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

}
