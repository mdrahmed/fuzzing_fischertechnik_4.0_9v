#include "TxtVacuumGripperRobot.h"

namespace ft {

TxtWPType_t TxtDeliveryPickupStation::getLastColor() {
    // Using static cast for sim simplicity to access parent VGR sim var or just return mock
    return WP_TYPE_NONE; 
}

TxtVacuumGripperRobot::TxtVacuumGripperRobot(TxtTransfer* pT, TxtMqttFactoryClient* mqttclient)
    : TxtSimulationModel(pT, mqttclient), mqttclient(mqttclient),
    currentState(__NO_STATE), newState(__NO_STATE),
    axisX("VGR_X", pT, 0, 0, 1500), axisY("VGR_Y", pT, 1, 1, 900), axisZ("VGR_Z", pT, 2, 2, 1100),
    vgripper(pT, 6, 7), dps(pT, mqttclient),
    reqQuit(false), reqOrder(false), reqNfcRead(false), reqNfcDelete(false),
    obs_vgr(0), obs_nfc(0), obs_dps(0), simDetectedColor(WP_TYPE_NONE)
{
    calibData.load();
    ord_state.type = WP_TYPE_NONE;
    ord_state.state = WAITING_FOR_ORDER;
}

TxtVacuumGripperRobot::~TxtVacuumGripperRobot() {
    delete obs_vgr; delete obs_nfc; delete obs_dps;
}

void TxtVacuumGripperRobot::setDetectedColor(TxtWPType_t c) { simDetectedColor = c; }

const char* TxtVacuumGripperRobot::toString(State_t state) {
    switch(state) {
        case IDLE: return "IDLE";
        case FETCH_WP_VGR: return "FETCH_WP_VGR";
        case STORE_WP_VGR: return "STORE_WP_VGR";
        case START_DELIVERY: return "START_DELIVERY";
        case COLOR_DETECTION: return "COLOR_DETECTION";
        case NFC_RAW: return "NFC_RAW";
        case STORE_WP: return "STORE_WP";
        case VGR_WAIT_FETCHED: return "VGR_WAIT_FETCHED";
        default: return "UNKNOWN";
    }
}

void TxtVacuumGripperRobot::requestOrder(TxtWPType_t type) {
    reqWP_order = TxtWorkpiece("TAG_ORDER", type, WP_STATE_RAW);
    reqOrder = true;
}

void TxtVacuumGripperRobot::move(const std::string pos3name, TxtVgrPosOrder_t order) {
    std::cout << "[VGR] Move to " << pos3name << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void TxtVacuumGripperRobot::move(uint16_t x, uint16_t y, uint16_t z, TxtVgrPosOrder_t order) {}
void TxtVacuumGripperRobot::moveRef() { std::cout << "[VGR] Homing...\n"; }
void TxtVacuumGripperRobot::stop() {}
void TxtVacuumGripperRobot::setSpeed(int16_t s) {}
void TxtVacuumGripperRobot::configInputs() {}
void TxtVacuumGripperRobot::initDashboard() {}

// Movement Sequences
void TxtVacuumGripperRobot::moveDeliveryInAndGrip() { move("DIN0"); move("DIN"); grip(); move("DIN0"); }
void TxtVacuumGripperRobot::moveDeliveryOutAndRelease() { move("DOUT0"); move("DOUT"); release(); moveRef(); }
void TxtVacuumGripperRobot::moveColorSensor(bool half) { move("DCS0"); if(!half) move("DCS"); }
void TxtVacuumGripperRobot::moveRefYNFC() { move("DNFC0"); move("DNFC"); }
void TxtVacuumGripperRobot::moveNFC() { move("DNFC0"); move("DNFC"); }
void TxtVacuumGripperRobot::moveWrongRelease() { move("WDC0"); move("WDC"); release(); moveRef(); }
void TxtVacuumGripperRobot::moveToHBW() { move("HBW0"); move("HBW"); }
void TxtVacuumGripperRobot::moveFromHBW1() { move("HBW0"); move("HBW"); }
void TxtVacuumGripperRobot::moveFromHBW2() { move("HBW1"); grip(); move("HBW"); move("HBW0"); }
void TxtVacuumGripperRobot::moveMPO() { move("MPO0"); move("MPO"); release(); }

}