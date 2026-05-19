import re
import json

def parse_model_profile(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Split by layers
    layers = re.split(r'\[LỚP \d+\]: ', content)[1:]
    
    blocks = {}
    
    for layer_content in layers:
        # Look for the Output tensor name and shape
        # Pattern: - Tensor[220] '...': Shape=[1, 28, 28, 192], DType=int8
        match = re.search(r"Outputs:.*?Tensor\[\d+\] '([^']+)': Shape=\[([^\]]+)\], DType=int8", layer_content, re.DOTALL)
        if not match:
            continue
        
        full_name = match.group(1)
        shape_str = match.group(2)
        shape = [int(s.strip()) for s in shape_str.split(',')]
        
        # Identify block name (e.g., block4a)
        block_match = re.search(r'block(\d+[a-z])', full_name)
        if not block_match:
            continue
        
        block_id = block_match.group(1)
        if block_id not in blocks:
            blocks[block_id] = {}
            
        # Parse based on layer type in the name
        if 'expand_conv' in full_name and 'CONV_2D' in layer_content:
            # Expansion layer
            # Input shape from the first input tensor
            input_match = re.search(r"Inputs:.*?Tensor\[\d+\] '[^']+': Shape=\[([^\]]+)\]", layer_content, re.DOTALL)
            if input_match:
                in_shape = [int(s.strip()) for s in input_match.group(1).split(',')]
                blocks[block_id]['PW_C_IN'] = in_shape[3]
                blocks[block_id]['PW_H_in'] = in_shape[1]
                blocks[block_id]['PW_W_in'] = in_shape[2]
            blocks[block_id]['PW_C_OUT'] = shape[3]
            
        elif 'dwconv2' in full_name and 'DEPTHWISE_CONV_2D' in layer_content:
            # Depthwise layer
            input_match = re.search(r"Inputs:.*?Tensor\[\d+\] '[^']+': Shape=\[([^\]]+)\]", layer_content, re.DOTALL)
            if input_match:
                in_shape = [int(s.strip()) for s in input_match.group(1).split(',')]
                stride = in_shape[1] // shape[1]
                blocks[block_id]['DW_STRIDE'] = stride
                
        elif 'se_reduce' in full_name and 'CONV_2D' in layer_content:
            blocks[block_id]['SE_PW_1_COUT'] = shape[3]
            
        elif 'project_conv' in full_name and 'CONV_2D' in layer_content:
            blocks[block_id]['PW_LAST_COUT'] = shape[3]

    # Filter out incomplete blocks (like fused ones that don't have DW)
    test_cases = []
    for b_id, params in sorted(blocks.items()):
        required = ['PW_C_IN', 'PW_C_OUT', 'PW_H_in', 'PW_W_in', 'DW_STRIDE', 'SE_PW_1_COUT', 'PW_LAST_COUT']
        if all(k in params for k in required):
            params['name'] = f"Block {b_id}"
            test_cases.append(params)
            
    return test_cases

if __name__ == "__main__":
    cases = parse_model_profile('model_profile.txt')
    print("test_cases = [")
    for case in cases:
        print(f"    {json.dumps(case)},")
    print("]")
