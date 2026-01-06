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
        std::cout << "INIT\n";
        moveRef();
        FSM_TRANSITION( IDLE );
        break;

    case IDLE:
        if (reqVGRfetchContainer) {
            FSM_TRANSITION( FETCH_CONTAINER );
            reqVGRfetchContainer = false;
        }
        else if (reqVGRfetch) {
            FSM_TRANSITION( FETCH_WP );
            reqVGRfetch = false;
        }
        else if (reqVGRcalib) {
            FSM_TRANSITION( CALIB_HBW );
            reqVGRcalib = false;
        }
        else if (reqVGRstore) {
            // Simplified logic: Direct to Store if no container fetch requested prior
            // Real logic might need FETCH_CONTAINER first, but for sim inputs:
            FSM_TRANSITION( STORE_WP );
        }
        else if (reqVGRresetStorage) {
            storage.resetStorageState();
            reqVGRresetStorage = false;
        }
        break;

    case FETCH_CONTAINER:
        std::cout << "FETCH_CONTAINER\n";
        if (fetchContainer()) {
            mqttclient->publishHBW_Ack(HBW_FETCHED, reqVGRwp, 100);
            FSM_TRANSITION( STORE_WP );
        } else {
            FSM_TRANSITION( FAULT );
        }
        break;

    case STORE_WP:
        std::cout << "STORE_WP\n";
        if (reqVGRstore) {
            if (reqVGRwp && store(*reqVGRwp)) {
                FSM_TRANSITION( IDLE );
            } else {
                std::cout << "Store Failed (Full?)\n";
                FSM_TRANSITION( FAULT );
            }
            reqVGRstore = false;
        }
        break;

    case FETCH_WP:
        std::cout << "FETCH_WP\n";
        if (reqVGRwp && fetch(reqVGRwp->type)) {
            mqttclient->publishHBW_Ack(HBW_FETCHED, reqVGRwp, 100);
            FSM_TRANSITION( FETCH_WP_WAIT );
        } else {
            std::cout << "Fetch Failed (Empty?)\n";
            FSM_TRANSITION( FAULT );
        }
        break;

    case FETCH_WP_WAIT:
        std::cout << "FETCH_WP_WAIT\n";
        if (reqVGRstoreContainer) {
            FSM_TRANSITION( STORE_CONTAINER );
            reqVGRstoreContainer = false;
        } else {
            // Sim: Auto return to IDLE if no container store requested
            FSM_TRANSITION( IDLE );
        }
        break;

    case STORE_CONTAINER:
        std::cout << "STORE_CONTAINER\n";
        if (storeContainer()) {
            FSM_TRANSITION( IDLE );
        } else {
            FSM_TRANSITION( FAULT );
        }
        break;

    case CALIB_HBW:
        std::cout << "CALIB_HBW\n";
        calibPos = (TxtHbwCalibPos_t)0;
        FSM_TRANSITION( CALIB_HBW_NAV );
        break;

    case CALIB_HBW_NAV:
        std::cout << "CALIB_HBW_NAV\n";
        // Mocking loop behavior for calibration
        moveCalibPos(); 
        mqttclient->publishHBW_Ack(HBW_CALIB_NAV, 0, 100);
        FSM_TRANSITION( CALIB_HBW_MOVE );
        break;

    case CALIB_HBW_MOVE:
        std::cout << "CALIB_HBW_MOVE\n";
        // Mocking manual movement loop
        moveJoystick();
        FSM_TRANSITION( CALIB_HBW_NAV ); 
        break;

    case FAULT:
        std::cout << "FAULT\n";
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