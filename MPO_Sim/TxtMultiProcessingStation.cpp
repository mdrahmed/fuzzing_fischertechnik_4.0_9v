#include "TxtMultiProcessingStation.h"

namespace ft {

TxtMultiProcessingStation::TxtMultiProcessingStation(TxtTransfer* pT, ft::TxtMqttFactoryClient* mqttclient)
    : TxtSimulationModel(pT, mqttclient), pT(pT), mqttclient(mqttclient),
      currentState(__NO_STATE), newState(__NO_STATE),
      chMsaw(1),
      vgripper(pT,7,8+4),
      axisGripper("gripper",pT,8+1,4,8+2),
      axisOvenInOut("ovenInOut",pT,8+0,8+1,8+0),
      axisRotTable("rotTable",pT,0,0,1,2),
      convBelt(pT,2),
      reqQuit(false), reqVGRwp(0), reqVGRproduce(false), reqSLDstarted(false),
      obs_mpo(0) // <--- FIX: Initialize pointer to nullptr
{
    calibData.load();
}

TxtMultiProcessingStation::~TxtMultiProcessingStation() {
    if (obs_mpo) {
        delete obs_mpo;
        obs_mpo = 0;
    }
}

const char* TxtMultiProcessingStation::toString(State_t state) {
    switch(state) {
        case IDLE: return "IDLE";
        case INIT: return "INIT";
        case BURN: return "BURN";
        case VGR_TRANSPORT: return "VGR_TRANSPORT";
        case TABLE_SAW: return "TABLE_SAW";
        case TABLE_BELT: return "TABLE_BELT";
        case EJECT: return "EJECT";
        case TRANSPORT: return "TRANSPORT";
        case FAULT: return "FAULT";
        default: return "UNKNOWN";
    }
}

bool TxtMultiProcessingStation::isEndConveyorBeltTriggered() {
    // Return true if triggered (simulated by helper)
    return simEndBeltTriggered;
}

void TxtMultiProcessingStation::setSawOff() { std::cout << "[Saw] OFF\n"; }
void TxtMultiProcessingStation::setSawLeft() { std::cout << "[Saw] LEFT\n"; }
void TxtMultiProcessingStation::setSawRight() { std::cout << "[Saw] RIGHT\n"; }
void TxtMultiProcessingStation::setValveEjection(bool on) { if(on) std::cout << "[Valve] Ejection ON\n"; }
void TxtMultiProcessingStation::setCompressor(bool on) { if(on) std::cout << "[Compressor] ON\n"; }

bool TxtMultiProcessingStation::isOvenTriggered() {
    return simOvenTriggered;
}

void TxtMultiProcessingStation::setValveVacuum(bool on) { if(on) std::cout << "[Valve] Vacuum ON\n"; }
void TxtMultiProcessingStation::setValveLowering(bool on) { if(on) std::cout << "[Valve] Lowering ON\n"; }
void TxtMultiProcessingStation::setValveOvenDoor(bool on) { if(on) std::cout << "[Valve] OvenDoor ON\n"; }
void TxtMultiProcessingStation::setLightOven(bool on) { if(on) std::cout << "[Light] Oven ON\n"; }

}