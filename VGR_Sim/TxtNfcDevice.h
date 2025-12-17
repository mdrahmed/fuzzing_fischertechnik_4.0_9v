#ifndef TXTNFCDEVICE_H_
#define TXTNFCDEVICE_H_

#include "SimulationMocks.h"

namespace ft {

typedef union ts_u { uint8_t u8[8]; int64_t s64; } uTS;
#define N_MAX_TS 8

class TxtNfcData {
public:
    TxtNfcData() : wp(), uts(), mask_ts(0) {}
    virtual ~TxtNfcData() {}
    TxtWorkpiece wp;
    uTS uts[N_MAX_TS];
    uint8_t mask_ts;
};

class TxtNfcDevice : public SubjectObserver {
public:
    TxtNfcDevice();
    virtual ~TxtNfcDevice();
    bool open() { return true; }
    void close() {}
    std::string readTagsGetUID();
    bool eraseTags();
    std::string readTags();
    bool writeTags(TxtWorkpiece wp, std::vector<uTS> vuts, uint8_t mask_ts);
    TxtNfcData* getNfcData() { return nfcData; }
    void publish() { Notify(); }
protected:
    TxtNfcData* nfcData;
    // Simulation Helper
    TxtWorkpiece simTag;
public: 
    void setSimTag(TxtWorkpiece wp) { simTag = wp; }
};

class TxtNfcDeviceObserver : public ft::Observer {
public:
    TxtNfcDeviceObserver(ft::TxtNfcDevice* s, ft::TxtMqttFactoryClient* m) : _subject(s), _mqttclient(m) { _subject->Attach(this); }
    virtual ~TxtNfcDeviceObserver() { _subject->Detach(this); }
    void Update(ft::SubjectObserver* o) {}
private:
    ft::TxtNfcDevice *_subject;
    ft::TxtMqttFactoryClient* _mqttclient;
};

}
#endif