# gen_seeds_store.py only for store functions of hbw
import os
os.makedirs("corpus", exist_ok=True)

# Directly from Config.HBW.Storage.json
real_workpieces = [
    # (type, state, uid)              # location
    (1, 0, b"043470a2186580"),        # A1
    (1, 0, b"044d64a2186580"),        # A2 / B1 (same uid — duplicate, intentional)
    (1, 0, b"04fe6ea2186580"),        # A3
    (2, 0, b"04a26ca2186580"),        # B2
    (2, 0, b"041467a2186580"),        # B3
    (2, 0, b"042f62a2186581"),        # C1
    (3, 0, b"042163a2186581"),        # C2
    (3, 0, b"044667a2186581"),        # C3
]

# Boundary/edge seeds based on real type range (1-3 seen, 0 absent)
edge_cases = [
    (0, 0, b"000000a2186580"),        # type=0 — NOT in real data, tests missing type
    (3, 2, b"044667a2186581"),        # C3 uid but state=2 (non-zero state)
    (0xFF, 0xFF, b"043470a2186580"),  # A1 uid with maxed-out bytes → wraps to type=3, state=0
    (2, 1, b"04a26ca2186580"),        # B2 uid with state=1
    (1, 0, b""),                      # empty uid — missing tag scenario
    (1, 0, b"\x00" * 14),            # null-filled uid same length as real uids
]

all_seeds = real_workpieces + edge_cases

for i, (t, s, uid) in enumerate(all_seeds):
    buf = bytes([t, s, len(uid)]) + uid
    with open(f"corpus/seed_{i:03d}", "wb") as f:
        f.write(buf)
    print(f"seed_{i:03d}: type={t}, state={s}, uid={uid}, total={len(buf)}B")

print("\nSeeds written to corpus/")