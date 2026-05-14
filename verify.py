import numpy as np
import subprocess
import os

def run_command(command):
    print(f"[EXEC] {command}")
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(f"[ERROR] {result.stderr}")
    return result.stdout

def save_to_dram(filename, data_list):
    with open(filename, 'w') as f:
        for val in data_list:
            f.write(f"{int(val)}\n")

def read_bram_output(filename, shape):
    data = []
    with open(filename, 'r') as f:
        for line in f:
            val = line.strip()
            if val:
                data.append(int(val))
    size = np.prod(shape)
    return np.array(data[:size]).reshape(shape)

# Config defaults
PW_C_IN = 96
PW_C_OUT = 384
PW_H = 14
PW_W = 14
DW_K = 3
SE_C = 24
PW_LAST_C = 96

# Generate Random Data
rng = np.random.default_rng(42)

ifm = rng.integers(-128, 127, size=(PW_H, PW_W, PW_C_IN), dtype=np.int8)
pw1_w = rng.integers(-128, 127, size=(PW_C_OUT, PW_C_IN), dtype=np.int8)
dw_w = rng.integers(-128, 127, size=(PW_C_OUT, DW_K, DW_K), dtype=np.int8)
se_pw1_w = rng.integers(-128, 127, size=(SE_C, PW_C_OUT), dtype=np.int8)
se_pw2_w = rng.integers(-128, 127, size=(PW_C_OUT, SE_C), dtype=np.int8)
pw_last_w = rng.integers(-128, 127, size=(PW_LAST_C, PW_C_OUT), dtype=np.int8)

# Pack into DRAM
# 0: IFM
# PW_WEIGHT_START_ADDR: PW1 Weights
# dw_start_addr: DW Weights
# se_pw_1_start_addr: SE PW1 Weights
# se_pw_2_start_addr: SE PW2 Weights
# pw_last_start_addr: PW LAST Weights

dram = np.zeros(1000000, dtype=np.int8)
idx = 0
dram[idx:idx+ifm.size] = ifm.flatten(); idx = ifm.size
# PW1 W: C_OUT * C_IN. In main.c it expects bram_indx * PW_FILTER_SIZE offset
# But in parse_args it updates PW_WEIGHT_START_ADDR = PW_C_IN * PW_H_in * PW_W_in;
pw_weight_start = PW_C_IN * PW_H * PW_W
for i in range(PW_C_OUT):
    # Each filter is loaded into a BRAM. bram_indx is i % 16? 
    # Actually main.c logic: 
    # for(int i = 0; i < PW_C_IN / BRAM_WIDTH_IN_BYTE; i++){
    #    load_bram(DRAM, PW_WEIGHT_START_ADDR + i * BRAM_WIDTH_IN_BYTE + bram_indx * PW_FILTER_SIZE, ...
    # PW_FILTER_SIZE is 96.
    # So filters are stored contiguously or with gaps? 
    # In main.c: bram_indx * PW_FILTER_SIZE. PW_FILTER_SIZE = PW_C_IN = 96.
    # So it's contiguous if BRAMs are filled one by one.
    pass

# Let's just follow main.c load logic to pack DRAM correctly
def pack_dram():
    d = np.zeros(1000000, dtype=np.int8)
    # IFM
    d[0:ifm.size] = ifm.flatten()
    
    # PW1 W
    pw_start = PW_C_IN * PW_H * PW_W
    for f in range(PW_C_OUT):
        # bram_indx = f % 16, but main.c does:
        # while(write_enable_weight != 0){ ... bram_indx = ...
        # and loops through tiles.
        # Actually PW1 load in main.c:
        # for(int tile = 0; tile < PW_NUM_OF_FILTER / NUM_OF_PE; tile++)
        # Inside compute loop it loads NEXT tile.
        # Initial load: 
        # while(write_enable_weight != 0){ 
        #    bram_indx = __builtin_ctz(write_enable_weight);
        #    load_bram(DRAM, PW_WEIGHT_START_ADDR + ... + bram_indx * PW_FILTER_SIZE, ...
        # This only loads the FIRST tile (16 filters).
        # The rest are loaded in the loop:
        # next_tile_offset = (tile + 1) * NUM_OF_PE * PW_FILTER_SIZE;
        # So weights are at PW_WEIGHT_START_ADDR + filter_idx * PW_C_IN
        d[pw_start : pw_start + pw1_w.size] = pw1_w.flatten()
    
    # DW W
    dw_start = pw_start + pw1_w.size
    dw_w_packed = dw_w.transpose(1, 2, 0) # (C, H, W) -> (H, W, C)
    d[dw_start : dw_start + dw_w.size] = dw_w_packed.flatten()
    
    # SE PW1 W
    se1_start = dw_start + dw_w.size
    d[se1_start : se1_start + se_pw1_w.size] = se_pw1_w.flatten()
    
    # SE PW2 W
    se2_start = se1_start + se_pw1_w.size
    d[se2_start : se2_start + se_pw2_w.size] = se_pw2_w.flatten()
    
    # PW LAST W
    pw_last_start = se2_start + se_pw2_w.size
    d[pw_last_start : pw_last_start + pw_last_w.size] = pw_last_w.flatten()
    
    return d

dram_data = pack_dram()
os.makedirs("test", exist_ok=True)
save_to_dram("test/dram.txt", dram_data)

# Run Simulation
run_command("gcc main.c -fopenmp -o main.exe")
if os.name == 'nt':
    run_command("main.exe")
else:
    run_command("./main.exe")

# Python Reference
def conv2d_pw(x, w):
    # x: (H, W, C_IN), w: (C_OUT, C_IN)
    h, w_in, c_in = x.shape
    c_out = w.shape[0]
    out = np.zeros((h, w_in, c_out), dtype=np.int32)
    # Using numpy for speed
    # x is (H*W, C_IN), w.T is (C_IN, C_OUT)
    x_flat = x.reshape(-1, c_in).astype(np.int32)
    w_t = w.astype(np.int32).T
    out_flat = np.dot(x_flat, w_t)
    return out_flat.reshape(h, w_in, c_out)

def conv2d_dw(x, w, padding=1):
    # x: (H, W, C), w: (C, K, K)
    h_in, w_in, c = x.shape
    k = w.shape[1]
    pad_h = k // 2
    x_padded = np.pad(x, ((pad_h, pad_h), (pad_h, pad_h), (0, 0)), mode='constant')
    out = np.zeros((h_in, w_in, c), dtype=np.int32)
    for i in range(c):
        for h in range(h_in):
            for w_idx in range(w_in):
                region = x_padded[h:h+k, w_idx:w_idx+k, i]
                out[h, w_idx, i] = np.sum(region.astype(np.int32) * w[i].astype(np.int32))
    return out

print("[REF] Computing PW1...")
pw1_out_32 = conv2d_pw(ifm, pw1_w)
pw1_out_8 = pw1_out_32.astype(np.int8)

print("[REF] Computing DW...")
dw_out_32 = conv2d_dw(pw1_out_8, dw_w)
dw_out_8 = dw_out_32.astype(np.int8)

print("[REF] Computing GAP...")
# Simulating integer division as in C (truncation towards zero)
gap_sums = np.sum(dw_out_8, axis=(0, 1)).astype(np.float64)
gap_out_sim = np.trunc(gap_sums / (PW_H * PW_W)).astype(np.int32)

print("[REF] Computing SE PW1...")
# gap_out_sim is (384,)
se1_out_32 = np.zeros(SE_C, dtype=np.int32)
for i in range(SE_C):
    se1_out_32[i] = np.sum(gap_out_sim.astype(np.int32) * se_pw1_w[i].astype(np.int32))
se1_out_8 = se1_out_32.astype(np.int8)

print("[REF] Computing SE PW2...")
se2_out_32 = np.zeros(PW_C_OUT, dtype=np.int32)
for i in range(PW_C_OUT):
    se2_out_32[i] = np.sum(se1_out_8.astype(np.int32) * se_pw2_w[i].astype(np.int32))
se2_out_8 = se2_out_32.astype(np.int8)

print("[REF] Computing MUL...")
mul_out_32 = dw_out_8.astype(np.int32) * se2_out_8.astype(np.int32)
mul_out_8 = mul_out_32.astype(np.int8)

print("[REF] Computing PW LAST...")
pw_last_out_32 = conv2d_pw(mul_out_8, pw_last_w)
pw_last_out_8 = pw_last_out_32.astype(np.int8)

print("[REF] Computing ADD...")
# C code: add_arr[i].c = input[i] + output[i]
# output[i] is (int8_t)temp[i]
final_out_32 = ifm.astype(np.int32) + pw_last_out_8.astype(np.int32)

# Compare
print("\n[COMPARE] Loading simulation outputs...")
sim_pw1 = read_bram_output("output/acc.txt", (PW_H, PW_W, PW_C_OUT))
sim_dw = read_bram_output("output/dw_acc.txt", (PW_H, PW_W, PW_C_OUT))
sim_gap = read_bram_output("output/gap_acc.txt", (PW_C_OUT,))
sim_se1 = read_bram_output("output/se_pw1.txt", (SE_C,))
sim_se2 = read_bram_output("output/se_pw2.txt", (PW_C_OUT,))
sim_mul = read_bram_output("output/mul.txt", (PW_H, PW_W, PW_C_OUT))
sim_pw_last = read_bram_output("output/pw_last_acc.txt", (PW_H, PW_W, PW_LAST_C))
sim_final = read_bram_output("output.txt", (PW_H, PW_W, PW_LAST_C))

def check(name, ref, sim):
    ref_cast = ref.astype(np.int32)
    sim_cast = sim.astype(np.int32)
    diff = np.abs(ref_cast - sim_cast)
    max_diff = np.max(diff)
    if max_diff == 0:
        print(f"[OK] {name} matches perfectly.")
    else:
        print(f"[FAIL] {name} has max diff {max_diff}")
        # Find first mismatch
        idx = np.unravel_index(np.argmax(diff), ref.shape)
        print(f"      First mismatch at {idx}: Ref={ref_cast[idx]}, Sim={sim_cast[idx]}")

check("PW1", pw1_out_32, sim_pw1)
check("DW", dw_out_32, sim_dw)
check("GAP", gap_out_sim, sim_gap)
check("SE1", se1_out_32, sim_se1)
check("SE2", se2_out_32, sim_se2)
check("MUL", mul_out_32, sim_mul)
check("PW_LAST", pw_last_out_32, sim_pw_last)
check("FINAL", final_out_32, sim_final)
