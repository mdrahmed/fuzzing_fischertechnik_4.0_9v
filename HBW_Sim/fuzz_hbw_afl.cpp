// fuzz_hbw_afl.cpp

#include "TxtHighBayWarehouse.h"
#include <cstdint>
#include <cstdlib>
#include <unistd.h>

// Required for AFL persistent mode
#ifdef __AFL_HAVE_MANUAL_CONTROL
#include <stdio.h>
#endif

// -------------------------------------------------------
// Input layout — MUST match seed generator
// -------------------------------------------------------
struct __attribute__((packed)) FuzzInput
{
    uint8_t cmds[16];
    uint8_t num_cmds;

    uint8_t wp_types[16];
    uint8_t wp_states[16];

    int32_t joy_aX1;
    int32_t joy_aY1;
    int32_t joy_aX2;
    int32_t joy_aY2;
    int32_t joy_b1;
    int32_t joy_b2;

    uint8_t num_fsm_steps;
};

// -------------------------------------------------------
// Core fuzz logic (reused per iteration)
// -------------------------------------------------------
static void run_input(const uint8_t *data, size_t size)
{
    if (size < sizeof(FuzzInput))
        return;

    const FuzzInput *in = reinterpret_cast<const FuzzInput *>(data);

    ft::TxtTransfer transfer;
    ft::TxtMqttFactoryClient mqtt;
    ft::TxtHighBayWarehouse hbw(&transfer, &mqtt);

    int num_cmds = (in->num_cmds % 16) + 1;
    int steps_per_cmd = (in->num_fsm_steps % 50) + 1;

    ft::TxtSimulationModel_status_t prev_status = hbw.getStatus();
    int stale_count = 0;

    for (int c = 0; c < num_cmds; c++)
    {
        uint8_t cmd = in->cmds[c] % 8;
        uint8_t wp_type = in->wp_types[c] % 8; // allow >3 → OOB exploration
        uint8_t wp_state = in->wp_states[c] % 3;

        ft::TxtWorkpiece wp(
            "TAG_FUZZ",
            static_cast<ft::TxtWPType_t>(wp_type),
            static_cast<ft::TxtWPState_t>(wp_state));

        switch (cmd)
        {
        case 0:
            hbw.requestVGRstore(&wp);
            break;
        case 1:
            hbw.requestVGRfetch(&wp);
            break;
        case 2:
            hbw.requestVGRfetchContainer(&wp);
            break;
        case 3:
            hbw.requestVGRstoreContainer(&wp);
            break;
        case 4:
            hbw.requestVGRcalib();
            break;
        case 5:
            hbw.requestVGRresetStorage();
            break;
        case 6:
            hbw.requestQuit();
            break;
        case 7:
        {
            ft::TxtJoysticksData jd;
            jd.aX1 = in->joy_aX1;
            jd.aY1 = in->joy_aY1;
            jd.aX2 = in->joy_aX2;
            jd.aY2 = in->joy_aY2;
            jd.b1 = in->joy_b1;
            jd.b2 = in->joy_b2;
            hbw.requestJoyBut(jd);
            break;
        }
        }

        for (int s = 0; s < steps_per_cmd; s++)
        {
            hbw.fsmStep();

            auto cur = hbw.getStatus();
            if (cur == prev_status)
            {
                if (++stale_count >= 3)
                    break;
            }
            else
            {
                stale_count = 0;
                prev_status = cur;
            }
        }

        // Direct calls → maximize bug exposure (important!)
        hbw.store(wp);
        hbw.fetch(static_cast<ft::TxtWPType_t>(wp_type));
        hbw.canColorBeStored(static_cast<ft::TxtWPType_t>(wp_type));
    }

    for (int i = 0; i < 10; i++)
    {
        hbw.fsmStep();
    }
}

// -------------------------------------------------------
// AFL++ entrypoint (persistent mode)
// -------------------------------------------------------
int main(int argc, char **argv)
{

#ifdef __AFL_HAVE_MANUAL_CONTROL
    __AFL_INIT();
#endif

    static uint8_t buf[4096];

    while (__AFL_LOOP(1000))
    { // persistent loop
        ssize_t len = read(0, buf, sizeof(buf));
        if (len <= 0)
            continue;

        run_input(buf, len);
    }

    return 0;
}