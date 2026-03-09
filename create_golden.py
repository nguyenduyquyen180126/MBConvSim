import torch
import torch.nn as nn
import numpy as np


if __name__ == '__main__':
    dw_ifm = np.ndarray((1, 384, 14, 14))
    with open('test/dw_ifm.txt', 'r') as f:
        for h in range(14):
            for w in range(14):
                for c in range(384):
                    dw_ifm[0, c, h, w] = int(f.readline().strip())

    dw_w = np.ndarray((384, 1, 3, 3))
    with open('test/w_3_3_384.txt') as f:
        for h in range(3):
            for w in range(3):
                for c_in in range(384):
                    dw_w[c_in, 0, h, w] = int(f.readline().strip())
    
    dw_conv = torch.nn.Conv2d(384, 384, 3, stride=1, padding='same', bias=False, groups=384)
    with torch.no_grad():
        dw_conv.weight.copy_(torch.from_numpy(dw_w).float())
    output = dw_conv(torch.from_numpy(dw_ifm).float())
    output = output.detach().numpy()
    output = output.astype(np.int32)
    _, C, H, W = output.shape
    with open('test/dw_acc.txt', 'w') as f:
        for h in range(H):
            for w in range(W):
                for c in range(C):
                    print(output[0, c, h, w], file=f)