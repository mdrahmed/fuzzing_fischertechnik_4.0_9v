#include "TxtSortingLine.h"

namespace ft {

bool TxtSortingLineCalibData::load() {
    // Hardcoded calibration for simulation
    color_th[0] = 940; // Threshold White-Red
    color_th[1] = 1430; // Threshold Red-Blue
    count_white = 5;
    count_red = 15;
    count_blue = 26;
    return true;
}

bool TxtSortingLineCalibData::saveDefault() { return true; }
bool TxtSortingLineCalibData::save() { return true; }

}