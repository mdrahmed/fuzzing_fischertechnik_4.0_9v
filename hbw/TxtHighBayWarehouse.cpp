#include "TxtHighBayWarehouse.h"

// FSM Logic Macros
#define FSM_INIT_FSM( startState, attr... ) currentState = startState; newState = startState;
#define FSM_TRANSITION( _newState, attr... ) do { \
    std::cout << "FSM: " << toString(currentState) << " -> " << toString(_newState) << "\n"; \
    newState = _newState; } while(0)

namespace ft {

// --- CalibData Implementation ---
bool TxtHighBayWarehouseCalibData::load() {
    hbx[0] = 780; hbx[1] = 1390; hbx[2] = 1995;
    hby[0] = 80;  hby[1] = 445;  hby[2] = 855;
    conv = EncPos2(20, 720);
    return true;
}
bool TxtHighBayWarehouseCalibData::saveDefault() { return true; }
bool TxtHighBayWarehouseCalibData::save() { return true; }

// --- HBW Implementation ---

TxtHighBayWarehouse::TxtHighBayWarehouse(TxtTransfer* pT, ft::TxtMqttFactoryClient* mqttclient)
    : TxtSimulationModel(pT, mqttclient), mqttclient(mqttclient),
    currentState(__NO_STATE), newState(__NO_STATE),
    calibPos(HBWCALIB_CV),
    axisX("HBW_X", pT, 1, 4, 2050),
    axisY("HBW_Y", pT, 3, 7, 1050),
    axisZ("HBW_Z", pT, 2, 5, 6),
    convBelt(pT, 0, 0, 3),
    reqQuit(false), reqVGRwp(0),
    obs_hbw(0), obs_storage(0)
{
    calibData.load();
}

TxtHighBayWarehouse::~TxtHighBayWarehouse() {
    delete obs_hbw;
    delete obs_storage;
}

const char* TxtHighBayWarehouse::toString(State_t state) {
    switch(state) {
        case IDLE: return "IDLE";
        case INIT: return "INIT";
        case FAULT: return "FAULT";
        case FETCH_CONTAINER: return "FETCH_CONTAINER";
        case STORE_WP: return "STORE_WP";
        case FETCH_WP: return "FETCH_WP";
        case FETCH_WP_WAIT: return "FETCH_WP_WAIT";
        case STORE_CONTAINER: return "STORE_CONTAINER";
        case CALIB_HBW: return "CALIB_HBW";
        default: return "UNKNOWN";
    }
}

void TxtHighBayWarehouse::stop() {
    axisX.stop(); axisY.stop(); axisZ.stop();
}

void TxtHighBayWarehouse::moveRef() {
    setActStatus(true, SM_BUSY);
    axisZ.moveS1();
    std::thread tx = axisX.moveRefThread();
    std::thread ty = axisY.moveRefThread();
    tx.join(); ty.join();
    setActStatus(false, SM_READY);
}

void TxtHighBayWarehouse::moveJoystick() { /* Mocked */ }

EncPos2 TxtHighBayWarehouse::getPos2() { return {axisX.getPosAbs(), axisY.getPosAbs()}; }

EncPos2 TxtHighBayWarehouse::moveConv(bool stop) {
    if (!stop) axisZ.moveS1();
    EncPos2 pos2 = calibData.conv;
    std::thread tx = axisX.moveAbsThread(pos2.x);
    std::thread ty = axisY.moveAbsThread(pos2.y);
    tx.join(); ty.join();
    return pos2;
}

EncPos2 TxtHighBayWarehouse::moveCR(int i, int j) {
    axisZ.moveS1();
    EncPos2 pos2;
    pos2.x = calibData.hbx[i];
    pos2.y = calibData.hby[j];
    std::thread tx = axisX.moveAbsThread(pos2.x);
    std::thread ty = axisY.moveAbsThread(pos2.y);
    tx.join(); ty.join();
    return pos2;
}

bool TxtHighBayWarehouse::getCR(int iCol, int iRow) {
    EncPos2 p2 = moveCR(iCol,iRow);
    axisZ.moveS2();
    axisY.moveAbs(p2.y - ydelta);
    axisZ.moveS1();
    return true;
}

bool TxtHighBayWarehouse::putCR(int iCol, int iRow) {
    EncPos2 p2 = moveCR(iCol,iRow);
    axisY.moveAbs(p2.y - ydelta);
    axisZ.moveS2();
    axisY.moveAbs(p2.y + ydelta);
    axisZ.moveS1();
    return true;
}

bool TxtHighBayWarehouse::getConv(bool stop) {
    EncPos2 p2 = moveConv(stop);
    axisZ.moveS2();
    convBelt.moveIn();
    axisY.moveAbs(p2.y - ydelta);
    if (!stop) axisZ.moveS1();
    return true;
}

bool TxtHighBayWarehouse::putConv(bool stop) {
    EncPos2 p2 = moveConv();
    axisZ.moveS2();
    axisY.moveAbs(p2.y + ydelta);
    convBelt.moveOut();
    if (!stop) axisZ.moveS1();
    return true;
}

bool TxtHighBayWarehouse::store(TxtWorkpiece wp) {
    setActStatus(true, SM_BUSY);
    if (storage.store(wp)) {
        StoragePos2 p = storage.getNextStorePos();
        getConv(true);
        putCR(p.x,p.y);
        setActStatus(false, SM_READY);
        moveRef();
        return true;
    }
    setActStatus(false, SM_ERROR);
    return false;
}

bool TxtHighBayWarehouse::storeContainer() {
    setActStatus(true, SM_BUSY);
    if (storage.storeContainer()) {
        StoragePos2 p = storage.getNextStorePos();
        getConv(true);
        putCR(p.x,p.y);
        setActStatus(false, SM_READY);
        moveRef();
        return true;
    }
    setActStatus(false, SM_ERROR);
    return false;
}

bool TxtHighBayWarehouse::fetch(TxtWPType_t t) {
    setActStatus(true, SM_BUSY);
    if (storage.fetch(t)) {
        StoragePos2 p = storage.getNextFetchPos();
        getCR(p.x, p.y);
        putConv(true);
        setActStatus(false, SM_READY);
        return true;
    }
    setActStatus(false, SM_ERROR);
    return false;
}

bool TxtHighBayWarehouse::fetchContainer() {
    setActStatus(true, SM_BUSY);
    if (storage.fetchContainer()) {
        StoragePos2 p = storage.getNextFetchPos();
        getCR(p.x, p.y);
        putConv(true);
        setActStatus(false, SM_READY);
        return true;
    }
    setActStatus(false, SM_ERROR);
    return false;
}

bool TxtHighBayWarehouse::canColorBeStored(TxtWPType_t c) { return storage.canColorBeStored(c); }
void TxtHighBayWarehouse::setSpeed(int16_t s) { axisX.setSpeed(s); axisY.setSpeed(s); axisZ.setSpeed(s); }

// --- FSM Implementation ---
void TxtHighBayWarehouse::fsmStep() {
    // Entry
    if( newState != currentState ) {
        switch( newState ) {
            case FAULT:
                printEntryState(FAULT);
                setStatus(SM_ERROR);
                sound.error();
                break;
            case IDLE:
                printEntryState(IDLE);
                setSpeed(512);
                moveRef();
                setActStatus(false, SM_READY);
                publishStorage();
                break;
            default: break;
        }
        currentState = newState;
    }

    // Do
    switch( currentState ) {
    case FAULT:
        if (reqQuit) {
            setStatus(SM_READY);
            FSM_TRANSITION( IDLE );
            reqQuit = false;
        }
        break;
    case INIT:
        printState(INIT);
        moveRef();
        FSM_TRANSITION( IDLE );
        break;
    case IDLE:
        if (reqVGRfetchContainer) {
            FSM_TRANSITION( FETCH_CONTAINER );
            reqVGRfetchContainer = false;
        }
        else if (reqVGRfetch) {
            FSM_TRANSITION( FETCH_WP );
            reqVGRfetch = false;
        }
        else if (reqVGRstore) {
            // Need to fetch container first if we are storing?
            // Simplified for logic:
            FSM_TRANSITION( STORE_WP );
        }
        else if (reqVGRresetStorage) {
            storage.resetStorageState();
            reqVGRresetStorage = false;
        }
        break;
    case FETCH_CONTAINER:
        if (fetchContainer()) {
            mqttclient->publishHBW_Ack(HBW_FETCHED, reqVGRwp, 100);
            FSM_TRANSITION( STORE_WP );
        } else {
            FSM_TRANSITION( FAULT );
        }
        break;
    case STORE_WP:
        if (reqVGRstore) {
            if (reqVGRwp && store(*reqVGRwp)) {
                FSM_TRANSITION( IDLE );
            } else {
                FSM_TRANSITION( FAULT );
            }
            reqVGRstore = false;
        }
        break;
    case FETCH_WP:
        if (reqVGRwp && fetch(reqVGRwp->type)) {
            mqttclient->publishHBW_Ack(HBW_FETCHED, reqVGRwp, 100);
            FSM_TRANSITION( FETCH_WP_WAIT );
        } else {
            FSM_TRANSITION( FAULT );
        }
        break;
    case FETCH_WP_WAIT:
        if (reqVGRstoreContainer) {
            FSM_TRANSITION( STORE_CONTAINER );
            reqVGRstoreContainer = false;
        }
        break;
    case STORE_CONTAINER:
        if (storeContainer()) {
            FSM_TRANSITION( IDLE );
        } else {
            FSM_TRANSITION( FAULT );
        }
        break;
    default: break;
    }
}

void TxtHighBayWarehouse::run() {
    obs_hbw = new TxtHighBayWarehouseObserver(this, mqttclient);
    obs_storage = new TxtHighBayWarehouseStorageObserver(&storage, mqttclient);

    FSM_INIT_FSM(INIT);
    while (!m_stoprequested) {
        fsmStep();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

}