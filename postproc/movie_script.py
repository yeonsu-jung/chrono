# %%
import os
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from scipy.spatial.transform import Rotation
# from vapory import *
from functions import *

import tkinter as tk
from tkinter import filedialog

import sys
from os.path import dirname
sys.path.append('C:\\Users\\yjung\\Documents\\GitHub\\vapory')

from vapory import *
# add directory


# root.withdraw()

# %matplotlib qt
# %%
foldername = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha125.0_RandomRods_Alpha125_N392_Date2023-02-14_23-25-09_tstep_1.00simtime_1.00'
foldername = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha38.0_RandomRods_Alpha38_N119_Date2023-02-14_23-03-10_tstep_1.00simtime_1.00 (1)'
foldername = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50 (1)'
foldername = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha200.0_RandomRods_Alpha200_N628_Date2023-02-14_23-23-50_tstep_1.00simtime_1.00 (3)'
foldername = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha66.0_RandomRods_Alpha66_N207_Date2023-02-14_23-33-50_tstep_0.50simtime_0.50 (1)'
fname = foldername + '/sim_data.txt'
# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha200.0_RandomRods_Alpha200_N628_Date2023-02-14_23-23-50_tstep_1.00simtime_1.00/sim_data.txt'
num_atoms,num_time_steps,variables = check_dump_file(fname)
print(num_atoms,num_time_steps)
# root = tk.Tk()
# init_dir = change_path_across_platforms('C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/')
# filename = filedialog.askopenfilename(initialdir = init_dir)
# root.destroy()
# foldername = os.path.dirname(filename)
foldername = r'C:\Users\yjung\Dropbox (Harvard University)\Entangled\Sims\alpha76.0_RandomRods_Alpha76_N1193_Date2023-02-22_01-36-22_tstep_1.00simtime_1.00'

# foldername = 'C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/alpha38.0_RandomRods_Alpha38_N596_Date2023-02-22_01-17-05_tstep_1.00simtime_1.00 (1)'
# foldername = 'C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/alpha125.0_RandomRods_Alpha125_N392_Date2023-02-14_23-25-09_tstep_1.00simtime_1.00 (3)'
# %%
motion_chunks = read_data(foldername + '/sim_data.txt',0,100,10)
try:
    metadata = read_metadata(foldername + '/metadata.txt')
except:
    metadata = read_metadata(foldername + '/metadata.yaml')
rod_length = int(metadata['rod_length'])
rod_radius = int(metadata['rod_radius'])

# %% preprocess
def replace_nan_with_nan(chunks):
    for i,chunk in enumerate(chunks):
        # replace nan with nan
        new_chunk = []
        for line in chunk:
            line = [np.nan if 'nan' in x else x for x in line]
            new_chunk.append(line)
                        
        chunks[i] = new_chunk

replace_nan_with_nan(motion_chunks)


# %% check scene first
scene_to_test = 5
df0 = pd.DataFrame(np.array(motion_chunks[scene_to_test][5:],dtype=float),columns=motion_chunks[scene_to_test][4][2:])
# 50th row in df0
floor_height = df0.y.min()
lid_height = df0.y.max()
box_height = lid_height - floor_height

# %%
# indexing third to fifth column
centroids = df0.iloc[:-6,2:5].to_numpy()
quaternions = df0.iloc[:-6,8:12].to_numpy()
# %%
import sys
sys.path.append('C:\\libraries\\povray\\povray-console-bin64')
# %%
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
scene.render('ipython', width=400, height=300, antialiasing=0.0001)
# %%
sim_data_path = foldername + '/sim_data.txt'
num_atoms,num_time_steps,variables = check_dump_file(sim_data_path)
print(num_atoms,num_time_steps)

# %%
start_chunk = 0
last_chunk = 50000
chunks_to_skip = 100
num_chunks = int((last_chunk - start_chunk)/chunks_to_skip)
print(num_chunks)

# %%
import regex as re
# extract alpha and N from filename and store it to variables
# _,alpha,N = re.match(r'.*alpha(\d+.\d+)_RandomRods_Alpha(\d+.\d+)_N(\d+)_Date.*',fname).groups()
_,alpha,N = re.match(r'.*alpha(\d+.\d+)_RandomRods_Alpha(\d+)_N(\d+)_Date.*',sim_data_path).groups()
rod_radius = 1
rod_length = float(alpha)*2

# %%
contact_chunks = read_data(foldername + '/contacts.txt',start_chunk,last_chunk,chunks_to_skip)
# %%
motion_chunks = read_data(sim_data_path,start_chunk,last_chunk,chunks_to_skip)

# %% movie
dir_name = f'./{alpha}_{N}_{last_chunk}_{chunks_to_skip}'
if os.path.exists(dir_name):
    i = 1
    while os.path.exists(dir_name + f' ({i})'):
        i += 1
    dir_name = dir_name + f' ({i})'
os.mkdir(dir_name)
# %%
def my_float(field):
    if 'nan' in field:
        field = np.NaN
    return float(field)

def my_float_list(list):
    return [my_float(field) for field in list]

for i,chunk in enumerate(motion_chunks):
    df = [my_float_list(line) for line in chunk[5:]]
    df = np.array(df)
    # remove nan rows
    nan_rows = np.isnan(df).any(axis=1)
    df = df[~nan_rows]    

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
# run matlab script to make movie
# os.system(f'matlab -nodisplay -nosplash -nodesktop -r "make_video"')
os.system(f'"/Applications/MATLAB_R2021a.app/bin/matlab" -nodisplay -nosplash -nodesktop -r "make_video("{dir_name}")"')

# %%


# N = len(contact_chunks)
# df_list = []
# for i0,chunk in enumerate(contact_chunks):    
#     N = len(chunk)
#     temp = [i for i in range(N)]
#     for i1,row in enumerate(chunk):    
#         temp[i1] = row.split()
    
#     df_list.append(pd.DataFrame(temp,dtype=float))
#     print(i0)

# # %%
# import time
# t_start = time.time()
# motion_chunks = contact_chunks = split_rows(motion_chunks)
# contact_chunks = split_rows(contact_chunks)
# t_last = time.time()
# t_elapsed = t_last - t_start
# print(t_elapsed)
