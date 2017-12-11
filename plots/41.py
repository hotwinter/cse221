import matplotlib.pyplot as plt
import numpy as np
import re

def read_data(fname):
    xs = []
    ys = []
    with open(fname, 'rb') as f:
        s = f.read().strip().split("\n")

    for i in range(0, len(s), 3):
        x = re.findall('([0-9]+): ', s[i])[0]
        xs.append(int(x))
        y = re.findall('Average: ([0-9.]+)us', s[i + 1])[0]
        ys.append(float(y))
    return xs, ys

x, y = read_data("file_cache.txt")
fig, ax = plt.subplots()
plt.xlabel('size/Bytes')
plt.ylabel('time/us')
#ax.set_xscale('symlog', basex=2)

plt.plot(x, y)
plt.ylim(0, 50)

plt.axvline(x = 890 * 1024 * 1024, color='r', linestyle='-.')
#plt.axvline(x = 2 * 1024 * 1024, color='r', linestyle='-.')
#plt.legend(handles=handles, bbox_to_anchor=(1.1, 1))
plt.show()
