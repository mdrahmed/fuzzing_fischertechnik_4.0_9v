#ifndef TXTVACUUMGRIPPER_H_
#define TXTVACUUMGRIPPER_H_

#include "SimulationMocks.h"

namespace ft {

class TxtVacuumGripper {
public:
    TxtVacuumGripper(TxtTransfer* pT, uint8_t chComp, uint8_t chValve);
    virtual ~TxtVacuumGripper();

    void grip();
    void release();

protected:
    void setCompressor(bool on);
    TxtTransfer* pT;
    uint8_t chComp;
    uint8_t chValve;
};

}
#endif