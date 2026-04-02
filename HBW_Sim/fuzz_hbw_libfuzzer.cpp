#include "TxtHighBayWarehouse.h"
#include <cstdint>
#include <cstddef>

// -------------------------------------------------------
// Input layout (must match gen_seeds.py)
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
// libFuzzer entrypoint
// -------------------------------------------------------
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{

    if (size < sizeof(FuzzInput))
        return 0;

    const FuzzInput *in = reinterpret_cast<const FuzzInput *>(data);

    // Instantiate system exactly like main.cpp
    ft::TxtTransfer transfer;
    ft::TxtMqttFactoryClient mqtt;
    ft::TxtHighBayWarehouse hbw(&transfer, &mqtt);

    // Clamp values but allow exploration
    int num_cmds = (in->num_cmds % 16) + 1;
    int steps_per_cmd = (in->num_fsm_steps % 50) + 1;

    ft::TxtSimulationModel_status_t prev_status = hbw.getStatus();
    int stale_count = 0;

    for (int i = 0; i < num_cmds; i++)
    {

        uint8_t cmd = in->cmds[i] % 8;
        uint8_t wp_type = in->wp_types[i] % 8; // allow invalid → OOB paths
        uint8_t wp_state = in->wp_states[i] % 3;

        ft::TxtWorkpiece wp(
            "FUZZ",
            static_cast<ft::TxtWPType_t>(wp_type),
            static_cast<ft::TxtWPState_t>(wp_state));

        // ---- Drive public API (FSM triggers) ----
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

        // ---- Drive FSM (core logic) ----
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

        // ---- Direct function fuzzing (critical for coverage) ----
        // These hit internal logic bypassing FSM guards
        hbw.store(wp);
        hbw.fetch(static_cast<ft::TxtWPType_t>(wp_type));
        hbw.canColorBeStored(static_cast<ft::TxtWPType_t>(wp_type));

        // ---- Target known risky APIs (index-based) ----
        int idx_i = wp_type; // may exceed [0,2]
        int idx_j = wp_state;

        hbw.moveCR(idx_i, idx_j);
        hbw.getCR(idx_i, idx_j);
        hbw.putCR(idx_i, idx_j);
    }

    // Final FSM drain
    for (int i = 0; i < 10; i++)
    {
        hbw.fsmStep();
    }

    return 0;
}