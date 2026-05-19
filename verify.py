import numpy as np
import subprocess
import os
import sys

def run_command(command):
    print(f"[EXEC] {command}")
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    if result.stderr:
        print(f"[ERROR] {result.stderr}")
    return result.stdout

def save_to_dram(filename, data_list):
    with open(filename, 'w') as f:
        for val in data_list:
            f.write(f"{int(val)}\n")

def read_bram_output(filename, shape):
    data = []
    if not os.path.exists(filename):
        print(f"[WARNING] {filename} not found.")
        return np.zeros(shape)
    with open(filename, 'r') as f:
        for line in f:
            val = line.strip()
            if val:
                data.append(int(val))
    size = np.prod(shape)
    if len(data) < size:
        padding = [0] * (size - len(data))
        data.extend(padding)
    return np.array(data[:size]).reshape(shape)

def check(name, ref, sim):
    ref_cast = ref.astype(np.int32)
    sim_cast = sim.astype(np.int32)
    diff = np.abs(ref_cast - sim_cast)
    max_diff = np.max(diff)
    if max_diff == 0:
        return True
    else:
        print(f"  [FAIL] {name} has max diff {max_diff}")
        idx = np.unravel_index(np.argmax(diff), ref.shape)
        print(f"        First mismatch at {idx}: Ref={ref_cast[idx]}, Sim={sim_cast[idx]}")
        return False

def run_test(config):
    print(f"\n>>> Running Test: {config['name']} <<<")
    
    # Extract parameters
    c_in = config['PW_C_IN']
    c_exp = config['PW_C_OUT']
    h_in = config['PW_H_in']
    w_in = config['PW_W_in']
    stride = config['DW_STRIDE']
    se_c = config['SE_PW_1_COUT']
    c_out = config['PW_LAST_COUT']
    
    # Calculate derived parameters
    h_out = int(np.ceil(h_in / stride))
    w_out = int(np.ceil(w_in / stride))

    # Full set of arguments for main.exe
    full_config = {
        'PW_C_IN': c_in, 'PW_C_OUT': c_exp, 'PW_H_in': h_in, 'PW_W_in': w_in,
        'PW_H_out': h_in, 'PW_W_out': w_in, 'PW_FILTER_SIZE': c_in, 'PW_NUM_OF_FILTER': c_exp,
        'DW_H_IN': h_in, 'DW_W_IN': w_in, 'DW_C_IN': c_exp, 'DW_H_OUT': h_out, 'DW_W_OUT': w_out,
        'DW_C_OUT': c_exp, 'DW_NUM_OF_K': c_exp, 'DW_STRIDE': stride,
        'SE_PW_1_CIN': c_exp, 'SE_PW_1_COUT': se_c, 'SE_PW_2_CIN': se_c, 'SE_PW_2_COUT': c_exp,
        'PW_LAST_CIN': c_exp, 'PW_LAST_H': h_out, 'PW_LAST_W': w_out, 'PW_LAST_COUT': c_out
    }

    # Generate Random Data
    rng = np.random.default_rng(config.get('seed', 42))
    ifm = rng.integers(-16, 15, size=(h_in, w_in, c_in), dtype=np.int8)
    pw1_w = rng.integers(-4, 3, size=(c_exp, c_in), dtype=np.int8)
    dw_w = rng.integers(-4, 3, size=(c_exp, 3, 3), dtype=np.int8)
    se_pw1_w = rng.integers(-4, 3, size=(se_c, c_exp), dtype=np.int8)
    se_pw2_w = rng.integers(-4, 3, size=(c_exp, se_c), dtype=np.int8)
    pw_last_w = rng.integers(-4, 3, size=(c_out, c_exp), dtype=np.int8)

    # Pack DRAM
    dram = np.zeros(1000000, dtype=np.int8)
    pw_start = c_in * h_in * w_in
    dw_start = pw_start + c_exp * c_in
    se1_start = dw_start + c_exp * 3 * 3
    se2_start = se1_start + se_c * c_exp
    pw_last_start = se2_start + c_exp * se_c
    
    dram[0:ifm.size] = ifm.flatten()
    dram[pw_start:pw_start+pw1_w.size] = pw1_w.flatten()
    dram[dw_start:dw_start+dw_w.size] = dw_w.transpose(1, 2, 0).flatten()
    dram[se1_start:se1_start+se_pw1_w.size] = se_pw1_w.flatten()
    dram[se2_start:se2_start+se_pw2_w.size] = se_pw2_w.flatten()
    dram[pw_last_start:pw_last_start+pw_last_w.size] = pw_last_w.flatten()
    
    save_to_dram("test/dram.txt", dram)

    # Run Simulation and capture output
    cmd_args = " ".join([f"-{k} {v}" for k, v in full_config.items()])
    sim_stdout = run_command(f"main.exe {cmd_args}" if os.name == 'nt' else f"./main.exe {cmd_args}")

    # Parse BRAM Usage and Cycles
    usage = {}
    lines = sim_stdout.split('\n')
    for line in lines:
        line = line.strip()
        if ':' in line:
            parts = line.split(':')
            key = parts[0].strip()
            val_str = parts[1].strip().split(' ')[0]
            
            # BRAM usage parsing
            if 'rows' in line:
                try:
                    usage[key] = int(val_str)
                except:
                    if '/' in parts[1]:
                        vals = parts[1].replace('rows','').strip().split('/')
                        usage[f"{key} Ping"] = int(vals[0].strip())
                        usage[f"{key} Pong"] = int(vals[1].strip())
            
            # Cycle parsing
            elif 'Cycles' in line or 'TOTAL' in line or 'COMPUTE' in line:
                try:
                    usage[key] = int(val_str)
                except:
                    pass

    # Python Reference logic
    def conv2d_pw(x, w):
        h, w_in, c_in = x.shape
        c_out = w.shape[0]
        x_flat = x.reshape(-1, c_in).astype(np.int32)
        w_t = w.astype(np.int32).T
        # Pha test
        #w_t = w_t + 1
        # --------
        out_flat = np.dot(x_flat, w_t)
        return out_flat.reshape(h, w_in, c_out)

    def conv2d_dw(x, w, stride=1):
        h_in, w_in, c = x.shape
        k = 3
        h_out = int(np.ceil(h_in / stride))
        w_out = int(np.ceil(w_in / stride))
        total_pad_h = (h_out - 1) * stride + k - h_in
        pad_t = total_pad_h // 2; pad_b = total_pad_h - pad_t
        total_pad_w = (w_out - 1) * stride + k - w_in
        pad_l = total_pad_w // 2; pad_r = total_pad_w - pad_l
        x_padded = np.pad(x, ((pad_t, pad_b), (pad_l, pad_r), (0, 0)), mode='constant')
        out = np.zeros((h_out, w_out, c), dtype=np.int32)
        for i in range(c):
            for h in range(h_out):
                for w_idx in range(w_out):
                    region = x_padded[h*stride:h*stride+k, w_idx*stride:w_idx*stride+k, i]
                    out[h, w_idx, i] = np.sum(region.astype(np.int32) * w[i].astype(np.int32))
        return out

    pw1_out_32 = conv2d_pw(ifm, pw1_w); pw1_out_8 = pw1_out_32.astype(np.int8)
    dw_out_32 = conv2d_dw(pw1_out_8, dw_w, stride); dw_out_8 = dw_out_32.astype(np.int8)
    gap_sums = np.sum(dw_out_8, axis=(0, 1)).astype(np.float64)
    gap_out_sim = np.trunc(gap_sums / (h_out * w_out)).astype(np.int32)
    se1_out_32 = np.zeros(se_c, dtype=np.int32)
    for i in range(se_c): se1_out_32[i] = np.sum(gap_out_sim.astype(np.int32) * se_pw1_w[i].astype(np.int32))
    se1_out_8 = se1_out_32.astype(np.int8)
    se2_out_32 = np.zeros(c_exp, dtype=np.int32)
    for i in range(c_exp): se2_out_32[i] = np.sum(se1_out_8.astype(np.int32) * se_pw2_w[i].astype(np.int32))
    se2_out_8 = se2_out_32.astype(np.int8)
    mul_out_8 = (dw_out_8.astype(np.int32) * se2_out_8.astype(np.int32)).astype(np.int8)
    pw_last_out_32 = conv2d_pw(mul_out_8, pw_last_w); pw_last_out_8 = pw_last_out_32.astype(np.int8)
    if stride == 1 and c_in == c_out:
        ifm_flat = ifm.flatten(); pw_last_out_flat = pw_last_out_8.flatten(); limit = pw_last_out_flat.size
        ifm_padded = np.pad(ifm_flat, (0, limit - ifm_flat.size), mode='constant') if ifm_flat.size < limit else ifm_flat[:limit]
        final_out_32 = (ifm_padded.astype(np.int32) + pw_last_out_flat.astype(np.int32)).reshape(h_out, w_out, c_out)
    else: final_out_32 = pw_last_out_32

    sim_pw1 = read_bram_output("output/acc.txt", (h_in, w_in, c_exp))
    sim_dw = read_bram_output("output/dw_acc.txt", (h_out, w_out, c_exp))
    sim_gap = read_bram_output("output/gap_acc.txt", (c_exp,))
    sim_se1 = read_bram_output("output/se_pw1.txt", (se_c,))
    sim_se2 = read_bram_output("output/se_pw2.txt", (c_exp,))
    sim_mul = read_bram_output("output/mul.txt", (h_out, w_out, c_exp))
    sim_pw_last = read_bram_output("output/pw_last_acc.txt", (h_out, w_out, c_out))
    sim_final = read_bram_output("output.txt", (h_out, w_out, c_out))

    success = True
    success &= check("PW1", pw1_out_8, sim_pw1)
    success &= check("DW", dw_out_8, sim_dw)
    success &= check("GAP", gap_out_sim, sim_gap)
    success &= check("SE1", se1_out_8, sim_se1)
    success &= check("SE2", se2_out_8, sim_se2)
    success &= check("MUL", mul_out_8, sim_mul)
    success &= check("PW_LAST", pw_last_out_8, sim_pw_last)
    success &= check("FINAL", final_out_32.astype(np.int8), sim_final)
    
    if success: print("  [OK] Matches perfectly.")
    return success, usage

# Test cases for EfficientNetV2-B0 extracted from model_profile.txt
test_cases = [
    {"PW_C_IN": 48, "PW_H_in": 28, "PW_W_in": 28, "PW_C_OUT": 192, "DW_STRIDE": 2, "SE_PW_1_COUT": 12, "PW_LAST_COUT": 96, "name": "Block 4a"},
    {"PW_C_IN": 96, "PW_H_in": 14, "PW_W_in": 14, "PW_C_OUT": 384, "DW_STRIDE": 1, "SE_PW_1_COUT": 24, "PW_LAST_COUT": 96, "name": "Block 4b"},
    {"PW_C_IN": 96, "PW_H_in": 14, "PW_W_in": 14, "PW_C_OUT": 384, "DW_STRIDE": 1, "SE_PW_1_COUT": 24, "PW_LAST_COUT": 96, "name": "Block 4c"},
    {"PW_C_IN": 96, "PW_H_in": 14, "PW_W_in": 14, "PW_C_OUT": 576, "DW_STRIDE": 1, "SE_PW_1_COUT": 24, "PW_LAST_COUT": 112, "name": "Block 5a"},
    {"PW_C_IN": 112, "PW_H_in": 14, "PW_W_in": 14, "PW_C_OUT": 672, "DW_STRIDE": 1, "SE_PW_1_COUT": 28, "PW_LAST_COUT": 112, "name": "Block 5b"},
    {"PW_C_IN": 112, "PW_H_in": 14, "PW_W_in": 14, "PW_C_OUT": 672, "DW_STRIDE": 1, "SE_PW_1_COUT": 28, "PW_LAST_COUT": 112, "name": "Block 5c"},
    {"PW_C_IN": 112, "PW_H_in": 14, "PW_W_in": 14, "PW_C_OUT": 672, "DW_STRIDE": 1, "SE_PW_1_COUT": 28, "PW_LAST_COUT": 112, "name": "Block 5d"},
    {"PW_C_IN": 112, "PW_H_in": 14, "PW_W_in": 14, "PW_C_OUT": 672, "DW_STRIDE": 1, "SE_PW_1_COUT": 28, "PW_LAST_COUT": 112, "name": "Block 5e"},
    {"PW_C_IN": 112, "PW_H_in": 14, "PW_W_in": 14, "PW_C_OUT": 672, "DW_STRIDE": 2, "SE_PW_1_COUT": 28, "PW_LAST_COUT": 192, "name": "Block 6a"},
    {"PW_C_IN": 192, "PW_H_in": 7, "PW_W_in": 7, "PW_C_OUT": 1152, "DW_STRIDE": 1, "SE_PW_1_COUT": 48, "PW_LAST_COUT": 192, "name": "Block 6b"},
    {"PW_C_IN": 192, "PW_H_in": 7, "PW_W_in": 7, "PW_C_OUT": 1152, "DW_STRIDE": 1, "SE_PW_1_COUT": 48, "PW_LAST_COUT": 192, "name": "Block 6c"},
    {"PW_C_IN": 192, "PW_H_in": 7, "PW_W_in": 7, "PW_C_OUT": 1152, "DW_STRIDE": 1, "SE_PW_1_COUT": 48, "PW_LAST_COUT": 192, "name": "Block 6d"},
    {"PW_C_IN": 192, "PW_H_in": 7, "PW_W_in": 7, "PW_C_OUT": 1152, "DW_STRIDE": 1, "SE_PW_1_COUT": 48, "PW_LAST_COUT": 192, "name": "Block 6e"},
    {"PW_C_IN": 192, "PW_H_in": 7, "PW_W_in": 7, "PW_C_OUT": 1152, "DW_STRIDE": 1, "SE_PW_1_COUT": 48, "PW_LAST_COUT": 192, "name": "Block 6f"},
    {"PW_C_IN": 192, "PW_H_in": 7, "PW_W_in": 7, "PW_C_OUT": 1152, "DW_STRIDE": 1, "SE_PW_1_COUT": 48, "PW_LAST_COUT": 192, "name": "Block 6g"},
    {"PW_C_IN": 192, "PW_H_in": 7, "PW_W_in": 7, "PW_C_OUT": 1152, "DW_STRIDE": 1, "SE_PW_1_COUT": 48, "PW_LAST_COUT": 192, "name": "Block 6h"},
]

if __name__ == "__main__":
    import csv
    run_command("gcc -I./include src/*.c main.c -fopenmp -o main.exe")
    all_success = True
    global_usage = {}
    csv_results = []

    for test in test_cases:
        ok, usage = run_test(test)
        if not ok: all_success = False
        
        # Collect data for CSV
        config_str = ", ".join([f"{k}={v}" for k, v in test.items() if k != "name"])
        full_name = f"{test['name']} ({config_str})"
        row = {"Block Name": full_name}
        
        # Add usage metrics (BRAM and Cycles)
        row.update(usage)
        csv_results.append(row)

        for k, v in usage.items():
            if k not in global_usage or (isinstance(v, int) and v > global_usage.get(k, 0)):
                global_usage[k] = v
    
    # Write to CSV
    if csv_results:
        # Get all unique keys
        all_keys = set()
        for r in csv_results:
            all_keys.update(r.keys())
        
        # Logical column ordering grouped by Layer
        def sort_priority(key):
            k_upper = key.upper()
            if key == "Block Name": return (0, 0, key)
            
            # Layer mapping (Sequence: INIT -> PW1 -> DW -> GAP -> SE1 -> SE2 -> MUL -> PW_LAST -> ADD -> TOTAL)
            layers = [
                ("INIT", 1), ("IFM", 1),
                ("PW1", 2), ("PW ", 2),
                ("DW", 3),
                ("GAP", 4),
                ("SE1", 5),
                ("SE2", 6),
                ("MUL", 7),
                ("PW_LAST", 8), ("PW LAST", 8), ("PW_L", 8),
                ("OUTPUT", 9),
                ("ADD", 10),
                ("TOTAL", 11)
            ]
            
            layer_idx = 99
            # Match layer patterns, prioritizing more specific (longer) names
            for pattern, idx in sorted(layers, key=lambda x: len(x[0]), reverse=True):
                if pattern in k_upper:
                    layer_idx = idx
                    break
            
            # Metric sub-ordering: Load -> Cycles -> Weight -> Acc -> BRAM
            metric_idx = 5
            if "LOAD" in k_upper: metric_idx = 0
            elif "CYCLES" in k_upper: metric_idx = 1
            elif "WEIGHT" in k_upper or " W " in k_upper: metric_idx = 2
            elif "ACC" in k_upper: metric_idx = 3
            elif "BRAM" in k_upper: metric_idx = 4
            
            return (layer_idx, metric_idx, key)

        sorted_keys = sorted(list(all_keys), key=sort_priority)
        
        with open('bram_usage_report.csv', 'w', newline='') as f:
            dict_writer = csv.DictWriter(f, fieldnames=sorted_keys)
            dict_writer.writeheader()
            dict_writer.writerows(csv_results)
        print(f"\n[LOGS] Detailed performance and BRAM report saved to 'bram_usage_report.csv'")

    if all_success:
        print("\n" + "="*50)
        print("EFFICIENTNETV2-B0 GLOBAL BRAM USAGE SUMMARY")
        print("="*50)
        for k, v in sorted(global_usage.items()):
            print(f"{k.ljust(25)}: {v} rows")
        print("="*50)
        print("\n[ALL TESTS PASSED]")
    else:
        print("\n[SOME TESTS FAILED]")
        sys.exit(1)
