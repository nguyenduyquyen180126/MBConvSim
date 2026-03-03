import torch
import torch.nn as nn
from torchvision.models import efficientnet_b0
from torchao.quantization import quantize_, int8_dynamic_activation_int8_weight_eager

# Initialize model
model = efficientnet_b0(weights='IMAGENET1K_V1')
model.eval()

# Quantize using torchao
quantize_(model, int8_dynamic_activation_int8_weight_eager())

quantized_model = model

# Dictionary to store hook outputs
hook_data = {
    'input': None,
    'output': None,
    'weight': None,
    'bias': None
}

# Define hook function
def conv_hook(module, input, output):
    hook_data['input'] = input[0].detach().cpu()
    hook_data['output'] = output.detach().cpu()
    if hasattr(module, 'weight'):
        hook_data['weight'] = module.weight.detach().cpu()
    if hasattr(module, 'bias') and module.bias is not None:
        hook_data['bias'] = module.bias.detach().cpu()

# Find and register hook on 1x1 conv layer
def find_and_hook_1x1_conv(model, hook_fn):
    for name, module in model.named_modules():
        if isinstance(module, nn.Conv2d) and module.kernel_size == (1, 1):
            print(f"Hooking into: {name}")
            module.register_forward_hook(hook_fn)
            return module
    return None

# Register hook
target_layer = find_and_hook_1x1_conv(quantized_model, conv_hook)

# Create dummy input and run inference
dummy_input = torch.randn(1, 3, 224, 224)

with torch.no_grad():
    output = quantized_model(dummy_input)

# Print captured data shapes
print(f"Input shape: {hook_data['input'].shape}")
print(f"Output shape: {hook_data['output'].shape}")
print(f"Weight shape: {hook_data['weight'].shape}")
print(f"Bias shape: {hook_data['bias'].shape if hook_data['bias'] is not None else 'None'}")