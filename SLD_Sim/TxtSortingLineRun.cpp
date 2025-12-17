#include "TxtSortingLine.h"

// FSM Logic Macros
#define FSM_INIT_FSM( startState, attr... ) currentState = startState; newState = startState;
#define FSM_TRANSITION( _newState, attr... ) do { \
    std::cout << "FSM: " << toString(currentState) << " -> " << toString(_newState) << "\n"; \
    newState = _newState; } while(0)

namespace ft {

void TxtSortingLine::fsmStep()
{
    // Entry Activities
    if( newState != currentState )
    {
        switch( newState )
        {
        case FAULT:
            printEntryState(FAULT);
            setStatus(SM_ERROR);
            sound.error();
            break;
        case IDLE:
            printEntryState(IDLE);
            setActStatus(false, SM_READY);
            simInputColor = WP_TYPE_NONE; // Reset after processing
            break;
        default: break;
        }
        currentState = newState;
    }

    // Do Activities
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
        FSM_TRANSITION( IDLE );
        break;

    case IDLE:
        // In sim: isColorSensorTriggered returns true if we set an input via main.cpp
        if (isColorSensorTriggered()) {
            FSM_TRANSITION( START );
        }
        else if (reqVGRcalib) {
            FSM_TRANSITION( CALIB_SLD );
            reqVGRcalib = false;
        }
        break;

    case START:
        setActStatus(true, SM_BUSY);
        convBelt.moveRight();
        detectedColorValue = 3000;
        FSM_TRANSITION( COLOR_DETECTION );
        break;

    case COLOR_DETECTION:
        if (readColorValue() < detectedColorValue) {
            detectedColorValue = lastColorValue;
        }
        if (isEjectionTriggered()) {
            std::cout << "Color Detected Value: " << detectedColorValue << "\n";
            FSM_TRANSITION( START_COUNT );
        }
        break;

    case START_COUNT:
        setCompressor(true);
        u16Counter = 0;
        FSM_TRANSITION( CHECK_COUNT );
        break;

    case CHECK_COUNT:
        // Simulate encoder counting up to target
        u16Counter++; 
        
        switch (getDetectedColor())
        {
        case WP_TYPE_WHITE:
            if (u16Counter >= calibData.count_white) 
                FSM_TRANSITION( EJECTION_WHITE );
            break;
        case WP_TYPE_RED:
            if (u16Counter >= calibData.count_red) 
                FSM_TRANSITION( EJECTION_RED );
            break;
        case WP_TYPE_BLUE:
            if (u16Counter >= calibData.count_blue) 
                FSM_TRANSITION( EJECTION_BLUE );
            break;
        default:
            if (u16Counter >= COUNT_WRONG) 
                FSM_TRANSITION( FAULT );
            break;
        }
        break;

    case EJECTION_WHITE:
        ejectWhite();
        FSM_TRANSITION( SORTED );
        break;

    case EJECTION_RED:
        ejectRed();
        FSM_TRANSITION( SORTED );
        break;

    case EJECTION_BLUE:
        ejectBlue();
        FSM_TRANSITION( SORTED );
        break;

    case SORTED:
        convBelt.stop();
        setActStatus(false, SM_READY);
        mqttclient->publishSLD_Ack(SLD_SORTED, getDetectedColor(), lastColorValue, 100);
        FSM_TRANSITION( IDLE );
        break;

    case CALIB_SLD:
        FSM_TRANSITION( IDLE ); // Simplified for simulation
        break;

    default: break;
    }
}

void TxtSortingLine::run()
{
    obs_sld = new TxtSortingLineObserver(this, mqttclient);
    FSM_INIT_FSM( INIT );
    
    while (!m_stoprequested)
    {
        fsmStep();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

}