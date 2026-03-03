import random
with open('dram.txt', 'w') as f:
    for i in range(128*12):
        print(random.randint(-128, 127), file=f)