#include "TxtHighBayWarehouse.h"

namespace ft
{

    TxtHighBayWarehouse::TxtHighBayWarehouse(TxtTransfer *pT, ft::TxtMqttFactoryClient *mqttclient)
        : TxtSimulationModel(pT, mqttclient), mqttclient(mqttclient),
          currentState(__NO_STATE), newState(__NO_STATE),
          axisX("HBW_X", pT, 1, 4, 2050),
          axisY("HBW_Y", pT, 3, 7, 1050),
          axisZ("HBW_Z", pT, 2, 5, 6),
          convBelt(pT, 0, 0, 3),
          reqQuit(false), reqVGRwp(0), reqVGRfetchContainer(false), reqVGRstore(false),
          reqVGRfetch(false), reqVGRstoreContainer(false), reqVGRcalib(false), reqVGRresetStorage(false),
          joyData(), reqJoyData(false),
          obs_hbw(0), obs_storage(0)
    {
        calibData.load();
    }

    TxtHighBayWarehouse::~TxtHighBayWarehouse()
    {
        delete obs_hbw;
        delete obs_storage;
    }

    const char *TxtHighBayWarehouse::toString(State_t state)
    {
        switch (state)
        {
        case IDLE:
            return "IDLE";
        case INIT:
            return "INIT";
        case FAULT:
            return "FAULT";
        case FETCH_CONTAINER:
            return "FETCH_CONTAINER";
        case STORE_WP:
            return "STORE_WP";
        case FETCH_WP:
            return "FETCH_WP";
        case FETCH_WP_WAIT:
            return "FETCH_WP_WAIT";
        case STORE_CONTAINER:
            return "STORE_CONTAINER";
        case CALIB_HBW:
            return "CALIB_HBW";
        case CALIB_HBW_NAV:
            return "CALIB_HBW_NAV";
        case CALIB_HBW_MOVE:
            return "CALIB_HBW_MOVE";
        default:
            return "UNKNOWN";
        }
    }

    void TxtHighBayWarehouse::requestJoyBut(TxtJoysticksData jd)
    {
        joyData = jd;
        reqJoyData = true;
    }

    void TxtHighBayWarehouse::stop()
    {
        axisX.stop();
        axisY.stop();
        axisZ.stop();
    }

    void TxtHighBayWarehouse::moveRef()
    {
        std::cout << "[HBW] Homing...\n";
        setActStatus(true, SM_BUSY);
        axisZ.moveS1();
        std::thread tx = axisX.moveRefThread();
        std::thread ty = axisY.moveRefThread();
        tx.join();
        ty.join();
        setActStatus(false, SM_READY);
    }

    void TxtHighBayWarehouse::moveJoystick()
    {
        if (reqJoyData)
        {
            std::cout << "[HBW] Joystick Move: " << joyData.aX1 << "\n";
            reqJoyData = false;
        }
    }

    EncPos2 TxtHighBayWarehouse::getPos2() { return {axisX.getPosAbs(), axisY.getPosAbs()}; }

    EncPos2 TxtHighBayWarehouse::moveConv(bool stop)
    {
        if (!stop)
            axisZ.moveS1();
        EncPos2 pos2 = calibData.conv;
        std::thread tx = axisX.moveAbsThread(pos2.x);
        std::thread ty = axisY.moveAbsThread(pos2.y);
        tx.join();
        ty.join();
        return pos2;
    }

    EncPos2 TxtHighBayWarehouse::moveCR(int i, int j)
    {
        axisZ.moveS1();
        EncPos2 pos2;
        pos2.x = calibData.hbx[i];
        pos2.y = calibData.hby[j];
        std::thread tx = axisX.moveAbsThread(pos2.x);
        std::thread ty = axisY.moveAbsThread(pos2.y);
        tx.join();
        ty.join();
        return pos2;
    }

    bool TxtHighBayWarehouse::getCR(int i, int j)
    {
        moveCR(i, j);
        axisZ.moveS2();
        axisY.moveAbs(calibData.hby[j] - ydelta);
        axisZ.moveS1();
        return true;
    }

    bool TxtHighBayWarehouse::putCR(int i, int j)
    {
        moveCR(i, j);
        axisY.moveAbs(calibData.hby[j] - ydelta);
        axisZ.moveS2();
        axisY.moveAbs(calibData.hby[j] + ydelta);
        axisZ.moveS1();
        return true;
    }

    bool TxtHighBayWarehouse::getConv(bool stop)
    {
        EncPos2 p2 = moveConv(stop);
        axisZ.moveS2();
        convBelt.moveIn();
        axisY.moveAbs(p2.y - ydelta);
        if (!stop)
            axisZ.moveS1();
        return true;
    }

    bool TxtHighBayWarehouse::putConv(bool stop)
    {
        EncPos2 p2 = moveConv();
        axisZ.moveS2();
        axisY.moveAbs(p2.y + ydelta);
        convBelt.moveOut();
        if (!stop)
            axisZ.moveS1();
        return true;
    }

    bool TxtHighBayWarehouse::store(TxtWorkpiece wp)
    {
        setActStatus(true, SM_BUSY);
        if (storage.store(wp))
        {
            StoragePos2 p = storage.getNextStorePos();
            getConv(true);
            putCR(p.x, p.y);
            setActStatus(false, SM_READY);
            moveRef();
            return true;
        }
        setActStatus(false, SM_ERROR);
        return false;
    }

    bool TxtHighBayWarehouse::storeContainer() { return true; }

    bool TxtHighBayWarehouse::fetch(TxtWPType_t t)
    {
        setActStatus(true, SM_BUSY);
        if (storage.fetch(t))
        {
            StoragePos2 p = storage.getNextFetchPos();
            getCR(p.x, p.y);
            putConv(true);
            setActStatus(false, SM_READY);
            return true;
        }
        setActStatus(false, SM_ERROR);
        return false;
    }

    bool TxtHighBayWarehouse::fetchContainer() { return true; }
    bool TxtHighBayWarehouse::canColorBeStored(TxtWPType_t c) { return storage.canColorBeStored(c); }
    void TxtHighBayWarehouse::setSpeed(int16_t s)
    {
        axisX.setSpeed(s);
        axisY.setSpeed(s);
        axisZ.setSpeed(s);
    }

    void TxtHighBayWarehouse::moveCalibPos()
    {
        // Simulating calibration move
        std::cout << "[HBW] Moving to Calib Pos: " << (int)calibPos << "\n";
        moveRef();
    }

}