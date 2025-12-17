#include "TxtVacuumGripperRobot.h"

namespace ft {

bool TxtVacuumGripperRobotCalibData::load() {
    // Mock calibration data
    setPos3("DIN0", EncPos3(22, 600, 19));
    setPos3("DIN", EncPos3(22, 758, 19));
    setPos3("HBW0", EncPos3(1410, 0, 0));
    setPos3("HBW", EncPos3(1410, 50, 186));
    setPos3("HBW1", EncPos3(1410, 170, 186));
    setPos3("MPO0", EncPos3(931, 0, 880));
    setPos3("MPO", EncPos3(931, 490, 934));
    setPos3("DCS0", EncPos3(123, 500, 65));
    setPos3("DCS", EncPos3(123, 645, 75));
    setPos3("DNFC0", EncPos3(200, 600, 250));
    setPos3("DNFC", EncPos3(200, 643, 260));
    setPos3("WDC0", EncPos3(300, 400, 0));
    setPos3("WDC", EncPos3(300, 600, 0));
    setPos3("DOUT0", EncPos3(263, 50, 280));
    setPos3("DOUT", EncPos3(263, 320, 590));
    return true;
}

}