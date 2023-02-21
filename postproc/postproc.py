# %%
# TO DO: write setup file
import os
import numpy as np
from matplotlib import pyplot as plt
from scipy.spatial.transform import Rotation
from vapory import *

# %matplotlib qt

# %%
def my_float(field):
    if 'nan' in field:
        field = np.NaN
    return float(field)

def my_float_list(list):
    return [my_float(field) for field in list]

def quaternion_multiply(q1, q2):
    w1, x1, y1, z1 = q1
    w2, x2, y2, z2 = q2
    w = w1*w2 - x1*x2 - y1*y2 - z1*z2
    x = w1*x2 + x1*w2 + y1*z2 - z1*y2
    y = w1*y2 - x1*z2 + y1*w2 + z1*x2
    z = w1*z2 + x1*y2 - y1*x2 + z1*w2
    return (w, x, y, z)

def quaternion_conjugate(q):
    w, x, y, z = q
    return (w, -x, -y, -z)

def quaternion_vector_rotation(q, v):
    qv = (0.0, *v)
    return quaternion_multiply(quaternion_multiply(q, qv), quaternion_conjugate(q))[1:]
    # return quaternion_multiply(quaternion_multiply(q, qv), q)[1:]

def find_lines_between_two_flags(filename, flag1, flag2):
    with open(filename) as f:
        # multiple checks?
        found_first_flag = False
        for i, line in enumerate(f):
            if line.startswith(flag2) & found_first_flag:
                end = i
                break
            if line.startswith(flag1):
                found_first_flag = True
                start = i
    
    return start, end

# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt'
# find_lines_between_two_flags(fname, 'ITEM: ATOMS', 'ITEM: TIME')

def read_next_line_after_flag(filename, flag):
    with open(filename) as f:
        while True:
            line = f.readline()
            if not line:
                break
            if line.startswith(flag):
                return f.readline()
            
def read_flag_line(filename, flag):
    with open(filename) as f:
        while True:
            line = f.readline()
            if not line:
                break
            if line.startswith(flag):
                return line

# %%
# Define a function to parse a LAMMPS plain-text dump file into a NumPy array
import subprocess

def count_lines(filename):
    output = subprocess.check_output(['wc', '-l', filename]).decode('utf-8')
    return int(output.split()[0])

def check_dump_file(filename):
    # information about the dump file
    # num of atoms
    # range of time frame
    # etc...
    num_atoms = read_next_line_after_flag(filename, 'ITEM: NUMBER')
    num_atoms = int(num_atoms)
    row1, row2 = find_lines_between_two_flags(filename, 'ITEM: TIME', 'ITEM: TIME')

    with open(filename, 'r') as f:
        f.readline()

    total_num_rows = count_lines(filename)

    variables = read_flag_line(filename, 'ITEM: ATOMS').split()[2:]
    num_time_steps = int(total_num_rows/(row2-row1))

    return num_atoms,num_time_steps,variables


# count_lines(fname)/num_atoms
# %%
# def read_lines_from_file(filename, start, end):
#     with open(filename) as file:
#         # Move to the start position
#         file.seek(start)
#         # Read the lines between start and end
#         lines = []
#         for i in range(start, end+1):
#             line = file.readline()
#             if not line:
#                 break
#             lines.append(line.rstrip('\n'))
#         return lines
    
def read_lines_from_file2(filename, start, end):
    with open(filename) as file:
        lines = []
        for i, line in enumerate(file):
            if i >= start:
                lines.append(line.rstrip('\n'))
            if i >= end:
                break
        return lines

# read_lines_from_file(fname, 2,10)
# read_lines_from_file2(fname, 0,5)

# %%
def get_data_from_dump(filename,last_chunk,chunks_to_skip):
    row1, row2 = find_lines_between_two_flags(filename, 'ITEM: TIME', 'ITEM: TIME')
    num_rows_in_chunk = row2-row1
    
    slices = [(i*num_rows_in_chunk,i*num_rows_in_chunk+num_rows_in_chunk-1) for i in range(0,last_chunk,chunks_to_skip)]    
    chunks = []
    for (s1,s2) in slices:
        # print(all_lines[s1:s2])
        chunk = []        
        for line in read_lines_from_file2(filename, s1, s2):
            # if line.startswith('ITEM:'):
            #     continue
            chunk.append((line.split()))
        chunks.append(chunk)
    return chunks

# %%
def read_inputs(foldername):
    with open(foldername + '/inputs.txt') as f:
        inputs = {}
        for line in f:
            key, value = line.split(' ')
            inputs[key] = value.strip()
    return inputs

import yaml
# read yaml file
def read_metadata(foldername):
    with open(foldername + '/metadata.yaml') as f:
        inputs = {}
        for line in f:
            key, value = line.split(' ')
            inputs[key] = value.strip()
    return inputs

foldername = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha125.0_RandomRods_Alpha125_N392_Date2023-02-14_23-25-09_tstep_1.00simtime_1.00'
inputs = read_inputs(foldername)
metadata = read_metadata(foldername)

# %% main script
# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt'
fname = foldername + '/sim_data.txt'
# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha200.0_RandomRods_Alpha200_N628_Date2023-02-14_23-23-50_tstep_1.00simtime_1.00/sim_data.txt'
num_atoms,num_time_steps,variables = check_dump_file(fname)
print(num_atoms,num_time_steps)

# %%
last_chunk = 1400
chunks_to_skip = 10
# %%
# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha200.0_RandomRods_Alpha200_N628_Date2023-02-14_23-23-50_tstep_1.00simtime_1.00/sim_data.txt'
import regex as re
# extract alpha and N from filename and store it to variables
_,alpha,N = re.match(r'.*alpha(\d+.\d+)_RandomRods_Alpha(\d+.\d+)_N(\d+)_Date.*',fname).groups()
# re.match(r'.*alpha(\d+.\d+)_RandomRods_Alpha(\d+.\d+)_N(\d+)_Date.*',fname).groups()
rod_radius = 1
rod_length = float(alpha)*2

# %%
# if system is windows
if os.name == 'nt':
    data, column_names, timesteps, num_atoms = parse_lammps_dump_file('C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt',rows_to_read=rows_to_read)
elif os.name == 'posix':
    # data, column_names, timesteps, num_atoms = parse_lammps_dump_file('/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt', rows_to_read=300000)
    data_chunks = get_data_from_dump(fname,last_chunk,chunks_to_skip)

# %%
# make a directory to store the images named after the simulation parameters
dir_name = f'./{alpha}_{N}'
# make a directory to store the images named after the simulation parameters
# if duplicate exists, add a number to the end of the directory name
dir_name = f'./{alpha}_{N}'
if os.path.exists(dir_name):
    i = 1
    while os.path.exists(dir_name + f'_{i}'):
        i += 1
    dir_name = dir_name + f'_{i}'
os.mkdir(dir_name)

# %%
import pandas as pd
# make data frame from data_chunks
# with delimeters ' '
df0 = pd.DataFrame(np.array(data_chunks[1][5:],dtype=float),columns=data_chunks[0][4][2:])
# 50th row in df0
floor_height = df0.y.min()
lid_height = df0.y.max()
box_height = lid_height - floor_height

# %%
# indexing third to fifth column
centroids = df0.iloc[:-6,2:5].to_numpy()
quaternions = df0.iloc[:-6,8:12].to_numpy()

camera_height = floor_height*1.5
camera = Camera( 'location', [0,camera_height,-box_height], 'look_at', [0,np.mean(centroids[1],axis=0),0] )
light = LightSource( [2,4,-box_height], 'color', [1,1,1] )
bgd = Background( 'color', [1,1,1] )
cylinders = []
for cen,q in zip(centroids,quaternions):
    ori = quaternion_vector_rotation(q, (0.0, 1.0, 0.0))
    base = [cen[0]-ori[0]*rod_length/2, cen[1]-ori[1]*rod_length/2, cen[2]-ori[2]*rod_length/2]
    cap = [cen[0]+ori[0]*rod_length/2, cen[1]+ori[1]*rod_length/2, cen[2]+ori[2]*rod_length/2]    
    cylinders.append(Cylinder( base, cap, rod_radius, Texture( Pigment( 'color', [1,0,1] ))))

scene = Scene( camera, objects= [light, *cylinders, bgd])
scene.render('ipython', width=800, height=600, antialiasing=0.0001)

    
# %%
for i,chunk in enumerate(data_chunks):
    df = [my_float_list(line) for line in chunk[5:]]
    df = np.array(df)

    cylinders = []
    for each_rod_data in df[:-6]:
        cen = each_rod_data[2:5]        
        quaternion = each_rod_data[8:12]
        ori = quaternion_vector_rotation(quaternion, (0.0, 1.0, 0.0))
        base = [cen[0]-ori[0]*rod_length/2, cen[1]-ori[1]*rod_length/2, cen[2]-ori[2]*rod_length/2]
        cap = [cen[0]+ori[0]*rod_length/2, cen[1]+ori[1]*rod_length/2, cen[2]+ori[2]*rod_length/2]
        
        cylinders.append(Cylinder( base, cap, rod_radius, Texture( Pigment( 'color', [1,0,1] ))))

    scene = Scene( camera, objects= [light, *cylinders, bgd])
    scene.render(f'{dir_name}/img_{i:04d}.png', width=800, height=600, antialiasing=0.0001)

# %%
# find index of 'fx' in variables
[variables.index('fx'),variables.index('fz')]

# %%
mu = []
force_vectors = []
for i,chunk in enumerate(data_chunks):
    df = [my_float_list(line.split()) for line in chunk[5:]]
    df = np.array(df)

    mu.append(np.mean(df[200,15:18]))
    force_vectors.append(df[200,15:18])

    # for each_rod_data in df:
    #     each_rod_data[15:18]
force_vectors = np.array(force_vectors)
# %%
plt.plot(mu)

# %%
# Define the time t
def vector_correlation(v,tau):
    # Calculate the dot products and sum
    n = len(v)
    out = np.zeros(n)
    for i in range(n):        
        out[i] = 1/n * np.sum(v[i:n] * v[0:n-i], axis=0).sum()
    return out

# %%
vc = vector_correlation(force_vectors,0)
print(vc)
# %%
plt.plot(vc)