# %%
# TO DO: write setup file

import numpy as np
from matplotlib import pyplot as plt
from scipy.spatial.transform import Rotation
%matplotlib qt
# %%
filename = "/Users/yjung/Documents/GitHub/generate_random_rods/test2.csv"

# %%
with open(filename) as f:
    lines = f.readlines()
# %%
N = len(lines)
# %%
all_rods = np.zeros((N,6))
for i,line in enumerate(lines):
    string_list = line.split(',')
    all_rods[i,:] = [float(string) for string in string_list]
    
        
# %%
fig = plt.figure(figsize = (8,8))
ax = plt.axes(projection='3d')
ax.grid()

for rod in all_rods:
    x = [rod[0], rod[3]]
    y = [rod[1], rod[4]]
    z = [rod[2], rod[5]]

    ax.plot3D(x, y, z,'o-')
    ax.set_title('3D Parametric Plot')
# %%
