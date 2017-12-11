import matplotlib.pyplot as plt
import numpy as np
import re

def read_data(fname):
    xs = []
    seq = []
    rand = []
    with open(fname, 'rb') as f:
        s = f.read().strip().split("\n")

    for i in range(0, 8):
        x = re.findall('([0-9]+): ', s[i * 3])[0]
        xs.append(x)
        y = re.findall('Average: ([0-9.]+)ms', s[i * 3 + 1])[0]
        seq.append(float(y))

    for i in range(8, 16):
        y = re.findall('Average: ([0-9.]+)ms', s[i * 3 + 1])[0]
        rand.append(float(y))
    return xs, seq, rand

x, seq, rand = read_data("remote_read.txt")
fig, ax = plt.subplots()
handles = []
plt.xlabel('Bytes read/B')
plt.ylabel('time/ms')
ax.set_xscale('symlog', basex=2)

handles.append(plt.plot(x, seq, label="random")[0])
handles.append(plt.plot(x, rand, label="sequential")[0])
plt.ylim(0, 30)

#plt.axvline(x = 800 * 1024 * 1024, color='r', linestyle='-.')
#plt.axvline(x = 2 * 1024 * 1024, color='r', linestyle='-.')
plt.legend(handles=handles, bbox_to_anchor=(1.1, 1))
plt.show()
