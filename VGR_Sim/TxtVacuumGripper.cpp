#include "TxtVacuumGripper.h"

namespace ft {

TxtVacuumGripper::TxtVacuumGripper(TxtTransfer* pT, uint8_t chComp, uint8_t chValve)
    : pT(pT), chComp(chComp), chValve(chValve)
{
    SPDLOG_LOGGER_DEBUG(spdlog::get("console"), "TxtVacuumGripper chComp:%d chValve:%d",  chComp, chValve);
}

TxtVacuumGripper::~TxtVacuumGripper() {}

void TxtVacuumGripper::grip() {
    std::cout << "[Gripper] Grip ON (Compressor ON, Valve Closed)\n";
    setCompressor(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduced for sim
}

void TxtVacuumGripper::release() {
    std::cout << "[Gripper] Release (Valve Open)\n";
    setCompressor(false);
}

void TxtVacuumGripper::setCompressor(bool on) {
    if (on) std::cout << "[Compressor] ON\n";
    else std::cout << "[Compressor] OFF\n";
}

}