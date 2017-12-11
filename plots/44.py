import matplotlib.pyplot as plt
import numpy as np
import re

def read_data(fname):
    seq = []
    rand = []
    with open(fname, 'rb') as f:
        s = f.read().strip().split("\n")

    for i in range(0, 8):
        y = re.findall('Average: ([0-9.]+)us', s[i * 3 + 1])[0]
        seq.append(float(y))

    for i in range(8, 16):
        y = re.findall('Average: ([0-9.]+)us', s[i * 3 + 1])[0]
        rand.append(float(y))
    return seq, rand

seq, rand = read_data("contention.txt")
fig, ax = plt.subplots()
x = range(1, 9)
handles = []
plt.xlabel('Number of processes')
plt.ylabel('time/us')
#ax.set_xscale('symlog', basex=2)

handles.append(plt.plot(x, seq, label="seqential")[0])
handles.append(plt.plot(x, rand, label="random")[0])

#plt.axvline(x = 800 * 1024 * 1024, color='r', linestyle='-.')
#plt.axvline(x = 2 * 1024 * 1024, color='r', linestyle='-.')
plt.legend(handles=handles, bbox_to_anchor=(1.1, 1))
plt.show()
