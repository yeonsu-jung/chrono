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

    arr = np.ndarray(data)

    return arr, column_names, timesteps, num_atoms
# %%
# Example usage: parse a LAMMPS plain-text dump file into a NumPy array and extract the simulation time
# arr, column_names, timesteps, num_atoms = parse_lammps_dump_file('C:/Users/yjung/Documents/GitHub/chrono/build/bin/Debug/output.txt')

import os

# if system is windows
if os.name == 'nt':
    arr, column_names, timesteps, num_atoms = parse_lammps_dump_file('C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt')
elif os.name == 'posix':
    arr, column_names, timesteps, num_atoms = parse_lammps_dump_file('/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt')

# print(f"Simulation time: {timestep}")
# %%
num_columns = np.min(arr.shape)
N = int(np.max(arr.shape)/num_atoms)
num_time_steps = len(timesteps)
assert(N == num_time_steps)
# %%
arr.shape

# %%
print(N,num_time_steps)

# %%
arr2 = np.reshape(arr, (num_time_steps, num_atoms, num_columns))

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
fname = 'C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt'
# read first 100 lines from fname
iter = 0
data = []
with open(fname) as f:
    for line in f:        
        if iter < 100 and iter > 5:
            # split line and take those in an numpy array
            try:
                data.append([my_float(k) for k in line.split()])

                # print(np.ndarray([my_float(k) for k in line.split()]))
                # data.append(np.ndarray([my_float(k) for k in line.split()]))
            except:
                1
                # print(line)
            
        iter += 1
# %%
data2 = np.array(data)
rod_positions = data2[:,2:5]
rod_orientations = data2[:,8:12]

# %%
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from scipy.spatial.transform import Rotation as R

# List of rod positions and orientations
# rod_positions = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
# rod_orientations = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0]]

# Create 3D plot
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Loop through each rod
for i in range(len(rod_positions)):
    # Define rod length and radius
    rod_length = 38
    rod_radius = 1

    # Get rod position and orientation
    rod_position = np.array(rod_positions[i])
    rod_orientation = np.array(rod_orientations[i])

    # Create rod axis
    rod_axis = np.array([1.0, 0.0, 0.0])
    rod_axis = R.from_quat(rod_orientation).apply(rod_axis)

    # Create rod points
    rod_points = np.array([rod_position + rod_axis * rod_length / 2.0,
                           rod_position - rod_axis * rod_length / 2.0])

    # Create rod surface
    u = np.linspace(0, 2 * np.pi, 100)
    v = np.linspace(0, np.pi, 100)
    x = rod_radius * np.outer(np.cos(u), np.sin(v))
    y = rod_radius * np.outer(np.sin(u), np.sin(v))
    z = rod_radius * np.outer(np.ones(np.size(u)), np.cos(v))

    # Rotate and translate rod surface to correct position
    R_matrix = R.from_rotvec(np.cross(np.array([1, 0, 0]), rod_axis)).as_matrix()
    x, y, z = np.dot(np.array([x.flatten(), y.flatten(), z.flatten()]).T, R_matrix).T.reshape(3, -1)
    x += rod_position[0]
    y += rod_position[1]
    z += rod_position[2]

    # Plot rod
    ax.plot(rod_points[:, 0], rod_points[:, 1], rod_points[:, 2], color='blue')
    ax.plot_surface(x.reshape(100, 100), y.reshape(100, 100), z.reshape(100, 100), color='blue')

# Set plot limits and labels
# ax.set_xlim3d(-10, 10)
# ax.set_ylim3d(-10, 10)
# ax.set_zlim3d(-10, 10)
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

# Show plot
plt.show()