import torch
import torch.nn as nn
import numpy as np


if __name__ == '__main__':
    rng = np.random.default_rng()



    gap_ifm = np.ndarray((1, 384, 14, 14))
    with open('test/dw_acc.txt', 'r') as f:
        for h in range(14):
            for w in range(14):
                for c in range(384):
                    gap_ifm[0, c, h, w] = int(f.readline().strip())
    gap_ifm = gap_ifm.astype(np.int8)
    output = torch.sum(torch.from_numpy(gap_ifm), dim=[2, 3], keepdim=True)
    output = output.numpy()
    output = output / (14*14)
    output = output.astype(np.int8)

    pw_conv = nn.Conv2d(384, 24, kernel_size=1, bias=False)
    pw_weight = rng.integers(-128, 127, size=(24, 384, 1, 1))
    with torch.no_grad():
        pw_conv.weight.copy_(torch.from_numpy(pw_weight).float())
    output = torch.from_numpy(output).float()
    output = pw_conv(output)
    output = output.detach().numpy()
    output = output.astype(np.int32)
    
    C_out, C_in, _, _ = pw_weight.shape
    with open('test/se_pw1_w_384_24.txt', 'w') as f:
        for cout in range(C_out):    
            for cin in range(C_in):
                print(pw_weight[cout, cin, 0, 0], file=f)
    # _, C, _, _ = output.shape
    # with open('test/se_pw1_out_1_1_24.txt', 'w') as f:
    #     for c in range(C):
    #         print(output[0, c, 0, 0], file=f)