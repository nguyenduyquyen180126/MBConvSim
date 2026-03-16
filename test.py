import torch
import torch.nn as nn
import numpy as np


if __name__ == '__main__':
    gap_ifm = np.ndarray((1, 384, 14, 14))
    with open('test/dw_acc.txt', 'r') as f:
        for h in range(14):
            for w in range(14):
                for c in range(384):
                    gap_ifm[0, c, h, w] = int(f.readline().strip())
    gap_ifm = gap_ifm.astype(np.int8)
    output = torch.sum(torch.from_numpy(gap_ifm), dim=[2, 3])
    output = output.numpy()
    _, C = output.shape
    with open('test/gap_acc.txt', 'w') as f:
        for c in range(C):
            print(output[0, c], file=f)
    with open('test/gap_out.txt', 'w') as f:
        for c in range(C):
            print(int(output[0, c] / (14*14)), file=f)