#include "TxtHighBayWarehouse.h"

namespace ft {

bool TxtHighBayWarehouseCalibData::load() {
    hbx[0] = 780; hbx[1] = 1390; hbx[2] = 1995;
    hby[0] = 80;  hby[1] = 445;  hby[2] = 855;
    conv = EncPos2(20, 720);
    return true;
}
bool TxtHighBayWarehouseCalibData::saveDefault() { return true; }
bool TxtHighBayWarehouseCalibData::save() { return true; }

}