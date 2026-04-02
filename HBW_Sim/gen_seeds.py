# gen_seeds.py
import struct, os
os.makedirs("seeds", exist_ok=True)

# struct layout: 16 cmds + num_cmds + 16 wp_types + 16 wp_states
#                + 5 int32 joy fields + num_fsm_steps
fmt = "16BB16B16BiiiiiB"

seeds = [
    # store white x3, then fetch white
    ([0,0,0,1]+[0]*12, 4, [1,1,1,1]+[0]*12, [0]*16, 0,0,0,0,0,0, 5),
    # store all colors
    ([0,0,0]+[0]*13,   3, [1,2,3]+[0]*13,   [0]*16, 0,0,0,0,0,0, 3),
    # calib sequence
    ([4,6]+[0]*14,     2, [0]*16,            [0]*16, 0,0,0,0,0,0, 10),
    # reset storage
    ([5]+[0]*15,       1, [0]*16,            [0]*16, 0,0,0,0,0,0, 2),
]

for i, (cmds, nc, wpt, wps, x1,y1,x2,y2,b1,b2, steps) in enumerate(seeds):
    # pack manually to match __attribute__((packed)) layout
    data = bytes(cmds) + bytes([nc]) + bytes(wpt) + bytes(wps)
    data += struct.pack("iiiii", x1, y1, x2, y2, b1)
    data += struct.pack("iB", b2, steps)
    open(f"seeds/seed_{i:03d}", "wb").write(data)
    print(f"seed_{i:03d}: {len(data)} bytes")