#include "TxtMultiProcessingStation.h"

// FSM Logic Macros
#define FSM_INIT_FSM( startState, attr... ) currentState = startState; newState = startState;
#define FSM_TRANSITION( _newState, attr... ) do { \
    std::cout << "FSM: " << toString(currentState) << " -> " << toString(_newState) << "\n"; \
    newState = _newState; } while(0)

namespace ft {

void TxtMultiProcessingStation::fsmStep()
{
    if( newState != currentState ) {
        currentState = newState;
    }

    switch( currentState )
    {
    case FAULT:
        if (reqQuit) {
            setStatus(SM_READY);
            FSM_TRANSITION( IDLE );
            reqQuit = false;
        }
        break;

    case INIT:
        setCompressor(true);
        setValveOvenDoor(true);
        axisOvenInOut.moveS1();
        axisRotTable.moveS1();
        axisGripper.moveS1();
        setCompressor(false);
        FSM_TRANSITION( IDLE );
        break;

    case IDLE:
        if (reqVGRproduce) {
            // Wait logic simulated: if not triggered, wait or timeout.
            // For sim, we assume triggered immediately or via helper
            if(isOvenTriggered()) {
                mqttclient->publishMPO_Ack(MPO_STARTED, 100);
                FSM_TRANSITION( BURN );
                reqVGRproduce = false;
                simOvenTriggered = false; // Reset
            }
        }
        break;

    case BURN:
        setCompressor(true);
        axisOvenInOut.moveS2(); // In
        setValveOvenDoor(false);
        // Burn loop simulation
        std::cout << "[Oven] Burning...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        setValveOvenDoor(true);
        axisOvenInOut.moveS1(); // Out
        FSM_TRANSITION( VGR_TRANSPORT );
        break;

    case VGR_TRANSPORT:
        axisRotTable.moveS1();
        setValveLowering(true);
        setValveVacuum(true);
        axisGripper.moveS1();
        setValveVacuum(false);
        setValveLowering(false);
        setCompressor(false);
        FSM_TRANSITION( TABLE_SAW );
        break;

    case TABLE_SAW:
        axisRotTable.moveS2();
        setSawRight();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        setSawOff();
        setSawLeft();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        setSawOff();
        FSM_TRANSITION( TABLE_BELT );
        break;

    case TABLE_BELT:
        axisRotTable.moveS3();
        FSM_TRANSITION( EJECT );
        break;

    case EJECT:
        convBelt.moveRight();
        setCompressor(true);
        setValveEjection(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        setValveEjection(false);
        setCompressor(false);
        mqttclient->publishMPO_Ack(MPO_PRODUCED, 100);
        FSM_TRANSITION( TRANSPORT );
        break;

    case TRANSPORT:
        // Wait for end trigger
        if(isEndConveyorBeltTriggered()) {
            std::cout << "[Transport] End Sensor Triggered\n";
            convBelt.stop();
            FSM_TRANSITION( IDLE );
            simEndBeltTriggered = false; // Reset
        }
        break;

    default: break;
    }
}

void TxtMultiProcessingStation::run()
{
    obs_mpo = new TxtMultiProcessingStationObserver(this, mqttclient);
    FSM_INIT_FSM( INIT );
    
    while (!m_stoprequested)
    {
        fsmStep();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

}