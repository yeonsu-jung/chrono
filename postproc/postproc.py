# %%
# TO DO: write setup file

import numpy as np
from matplotlib import pyplot as plt
from scipy.spatial.transform import Rotation
%matplotlib qt

# %%
def my_float(field):
    if 'nan' in field:
        field = np.NaN
    return float(field)

# %%
# Define a function to parse a LAMMPS plain-text dump file into a NumPy array
def parse_lammps_dump_file(filename):
    # Read the file and extract the header and data lines
    with open(filename) as f:
        lines = f.readlines()

    header_lines = [line for line in lines if line.startswith('ITEM: ATOMS')]
    header_positions = [i for i,line in enumerate(lines) if line.startswith('ITEM: ATOMS')]
    last_header_position = header_positions[-1]

    num_atoms = int([lines[i+1] for i,line in enumerate(lines) if line.startswith('ITEM: NUMBER')][0].split()[0])

    if (len(lines) - last_header_position != num_atoms + 1):
        print("Last timestep is incomplete")

    data_lines = [line for line in lines if (not line.startswith('ITEM:')) and line.find(' ') != -1]
    timesteps = [float(lines[i+1].split()[0]) for i,line in enumerate(lines) if line.startswith('ITEM: TIME')]

    # Extract the column names from the last header line
    header_line = header_lines[-1].split()[2:]
    column_names = ['id', 'type'] + header_line

    # Extract the timestep value from the "ITEM: TIMESTEP" header line
    # timestep_line = [line for line in lines if line.startswith('ITEM: TIMESTEP')]
    # print(timestep_line)
    # timestep = [line for line in lines if (not line.startswith('ITEM:')) and (line.find(' ') == -1)]    

    # Parse the data lines into a NumPy array
    data = []
    for line in data_lines:
        fields = line.split()
        data.append([my_float(field) for field in fields])

    arr = np.array(data)

    return arr, column_names, timesteps, num_atoms
# %%
# Example usage: parse a LAMMPS plain-text dump file into a NumPy array and extract the simulation time
arr, column_names, timesteps, num_atoms = parse_lammps_dump_file('C:/Users/yjung/Documents/GitHub/chrono/build/bin/Debug/output.txt')
# print(f"Simulation time: {timestep}")
# %%
num_columns = np.min(arr.shape)
N = int(np.max(arr.shape)/num_atoms)
num_time_steps = len(timesteps)
assert(N == num_time_steps)


# %%
arr2 = np.reshape(arr, (num_time_steps, num_atoms, num_columns))

# %%

# %%

fig = plt.figure(figsize = (8,8))
ax = plt.axes(projection='3d')
ax.grid()

x = arr2[10,:,2]
y = arr2[10,:,3]
z = arr2[10,:,4]

u1 = arr2[10,:,8]
u2 = arr2[10,:,9]
u3 = arr2[10,:,10]
u4 = arr2[10,:,11]

ax.plot3D(x, y, z,'o')
ax.set_title('3D Parametric Plot')

# Set axes label
ax.set_xlabel('x', labelpad=20)
ax.set_ylabel('y', labelpad=20)
ax.set_zlabel('t', labelpad=20)

plt.show()
# %%

qs = arr2[10,:,8:12]
r_list = [Rotation.from_quat(q) for q in qs]

rot_mat_list = [r.as_matrix() for r in r_list]

print(rot_mat_list)
# %%
print(np.mean(arr2[1000,:,15:18]))
# %%
