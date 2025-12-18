#include "TxtHighBayWarehouse.h"

// FSM Logic Macros
#define FSM_INIT_FSM( startState, attr... ) currentState = startState; newState = startState;
#define FSM_TRANSITION( _newState, attr... ) do { \
    std::cout << "FSM: " << toString(currentState) << " -> " << toString(_newState) << "\n"; \
    newState = _newState; } while(0)

namespace ft {

void TxtHighBayWarehouse::fsmStep()
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
        if (reqVGRstore) {
            if (reqVGRwp && store(*reqVGRwp)) {
                FSM_TRANSITION( IDLE );
            } else {
                std::cout << "Store Failed (Full?)\n";
                FSM_TRANSITION( FAULT );
            }
            reqVGRstore = false;
        }
        else if (reqVGRfetch) {
            if (reqVGRwp && fetch(reqVGRwp->type)) {
                FSM_TRANSITION( IDLE );
            } else {
                std::cout << "Fetch Failed (Empty?)\n";
                FSM_TRANSITION( FAULT );
            }
            reqVGRfetch = false;
        }
        break;

    case FAULT:
        if (reqQuit) {
            FSM_TRANSITION( IDLE );
            reqQuit = false;
        }
        break;

    default: break;
    }
}

void TxtHighBayWarehouse::run() {
    obs_hbw = new TxtHighBayWarehouseObserver(this, mqttclient);
    obs_storage = new TxtHighBayWarehouseStorageObserver(&storage, mqttclient);
    
    FSM_INIT_FSM( INIT );
    
    while (!m_stoprequested) {
        fsmStep();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

}