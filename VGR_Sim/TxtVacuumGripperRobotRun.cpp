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
        moveRef();
        FSM_TRANSITION( IDLE );
        break;

    case IDLE:
        // 1. Process "Store" Command (Simulating receiving item at input)
        // In sim, we set detected color externally, then trigger this path
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
        moveToHBW();
        // In real code, we ask HBW to fetch a container first.
        // Here we simulate the handshake immediately.
        reqHBWfetched = true; 
        FSM_TRANSITION( STORE_WP );
        break;

    case STORE_WP:
        if (reqHBWfetched) {
            release(); // Place item in HBW
            moveRef();
            // Signal to HBW logic (handled in main loop via array update)
            reqHBWfetched = false;
            simDetectedColor = WP_TYPE_NONE; // Reset input
            FSM_TRANSITION( IDLE );
        }
        break;

    case FETCH_WP_VGR:
        // Fetch specific type
        moveFromHBW1();
        // Simulating HBW providing the item
        reqHBWfetched = true; 
        FSM_TRANSITION( VGR_WAIT_FETCHED );
        break;

    case VGR_WAIT_FETCHED:
        if (reqHBWfetched) {
            moveFromHBW2(); // Pick up from HBW
            reqHBWfetched = false;
            // Move to MPO or Out
            moveMPO(); // Simplified: just drop off
            moveRef();
            FSM_TRANSITION( IDLE );
        }
        break;

    default: break;
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