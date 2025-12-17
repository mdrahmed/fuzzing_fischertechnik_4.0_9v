#include "TxtSortingLine.h"

namespace ft {

uint16_t u16LastState = 0;
uint16_t u16Counter = 0;

TxtSortingLine::TxtSortingLine(TxtTransfer* pT, ft::TxtMqttFactoryClient* mqttclient)
    : TxtSimulationModel(pT, mqttclient), mqttclient(mqttclient),
    currentState(__NO_STATE), newState(__NO_STATE),
    convBelt(pT, 0), chEW(4), chER(5), chEB(6), chComp(7),
    lastColorValue(-1), calibColor(ft::WP_TYPE_NONE), 
    reqQuit(false), reqMPOproduced(false), reqVGRstart(false), reqVGRcalib(false),
    obs_sld(0), simInputColor(WP_TYPE_NONE), simEjectionCounter(0)
{
    calibData.load();
}

TxtSortingLine::~TxtSortingLine() {
    delete obs_sld;
}

const char* TxtSortingLine::toString(State_t state) {
    switch(state) {
        case IDLE: return "IDLE";
        case START: return "START";
        case COLOR_DETECTION: return "COLOR_DETECTION";
        case START_COUNT: return "START_COUNT";
        case CHECK_COUNT: return "CHECK_COUNT";
        case EJECTION_WHITE: return "EJECTION_WHITE";
        case EJECTION_RED: return "EJECTION_RED";
        case EJECTION_BLUE: return "EJECTION_BLUE";
        case SORTED: return "SORTED";
        case FAULT: return "FAULT";
        default: return "UNKNOWN";
    }
}

// --- Simulation Helpers ---
void TxtSortingLine::setInputColor(TxtWPType_t color) {
    simInputColor = color;
    simEjectionCounter = 0;
}

// Mocks reading sensor input based on simulation state
bool TxtSortingLine::isColorSensorTriggered() {
    return (simInputColor != WP_TYPE_NONE && currentState == IDLE);
}

// Returns raw analog value based on color input
int TxtSortingLine::readColorValue() {
    if (simInputColor == WP_TYPE_WHITE) lastColorValue = 500;
    else if (simInputColor == WP_TYPE_RED) lastColorValue = 1200;
    else if (simInputColor == WP_TYPE_BLUE) lastColorValue = 1600;
    else lastColorValue = 2500; // None
    return lastColorValue;
}

bool TxtSortingLine::isEjectionTriggered() {
    // Simulate triggering ejection after color detection
    return (currentState == COLOR_DETECTION);
}

ft::TxtWPType_t TxtSortingLine::getLastColor() {
    if (lastColorValue < calibData.color_th[0]) return WP_TYPE_WHITE;
    if (lastColorValue < calibData.color_th[1]) return WP_TYPE_RED;
    if (lastColorValue < 2000) return WP_TYPE_BLUE;
    return WP_TYPE_NONE;
}

ft::TxtWPType_t TxtSortingLine::getDetectedColor() {
    if (detectedColorValue < calibData.color_th[0]) return WP_TYPE_WHITE;
    if (detectedColorValue < calibData.color_th[1]) return WP_TYPE_RED;
    if (detectedColorValue < 2000) return WP_TYPE_BLUE;
    return WP_TYPE_NONE;
}

void TxtSortingLine::ejectWhite() { 
    std::cout << "[Ejector] WHITE Ejected\n"; 
    setCompressor(false); 
}
void TxtSortingLine::ejectRed() { 
    std::cout << "[Ejector] RED Ejected\n"; 
    setCompressor(false); 
}
void TxtSortingLine::ejectBlue() { 
    std::cout << "[Ejector] BLUE Ejected\n"; 
    setCompressor(false); 
}

void TxtSortingLine::setCompressor(bool on) {
    if(on) std::cout << "[Compressor] ON\n";
}

}