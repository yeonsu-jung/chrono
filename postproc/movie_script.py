import os
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from scipy.spatial.transform import Rotation
from vapory import *
from functions import *
# %matplotlib qt
# %%
foldername = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha125.0_RandomRods_Alpha125_N392_Date2023-02-14_23-25-09_tstep_1.00simtime_1.00'
foldername = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha38.0_RandomRods_Alpha38_N119_Date2023-02-14_23-03-10_tstep_1.00simtime_1.00 (1)'
foldername = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50 (1)'
fname = foldername + '/sim_data.txt'
# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha200.0_RandomRods_Alpha200_N628_Date2023-02-14_23-23-50_tstep_1.00simtime_1.00/sim_data.txt'
num_atoms,num_time_steps,variables = check_dump_file(fname)
print(num_atoms,num_time_steps)

# %%
last_chunk = 32000
chunks_to_skip = 100
# %%
# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha200.0_RandomRods_Alpha200_N628_Date2023-02-14_23-23-50_tstep_1.00simtime_1.00/sim_data.txt'
import regex as re
# extract alpha and N from filename and store it to variables
# _,alpha,N = re.match(r'.*alpha(\d+.\d+)_RandomRods_Alpha(\d+.\d+)_N(\d+)_Date.*',fname).groups()
_,alpha,N = re.match(r'.*alpha(\d+.\d+)_RandomRods_Alpha(\d+)_N(\d+)_Date.*',fname).groups()
rod_radius = 1
rod_length = float(alpha)*2
# %%

# %%
# if system is windows
if os.name == 'nt':
    data, column_names, timesteps, num_atoms = parse_lammps_dump_file('C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt',rows_to_read=rows_to_read)
elif os.name == 'posix':
    # data, column_names, timesteps, num_atoms = parse_lammps_dump_file('/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt', rows_to_read=300000)
    motion_chunks = get_data_from_dump(fname,last_chunk,chunks_to_skip)
# %%
def read_contact_data(foldername,last_chunk,chunks_to_skip):
    filename = foldername + '/contacts.txt'
    chunks_to_read = range(0,last_chunk,chunks_to_skip)
    count = -1
    chunks = []
    with open(filename) as f:
        # whenever meets 'ITEM: TIME', count it and check if it's the chunk we want to read
        chunk = []
        for i, line in enumerate(f):
            if line.startswith('ITEM: TIME'):
                count += 1
                if count-1 in chunks_to_read:
                    chunks.append(chunk)
                    chunk = []
                if count in chunks_to_read:
                    chunk.append(line.rstrip('\n'))
            elif count in chunks_to_read:
                chunk.append(line.rstrip('\n'))
            
    return chunks

contact_chunks = read_contact_data(foldername,last_chunk,chunks_to_skip)
# %%
# inputs = read_inputs(foldername)
metadata = read_metadata(foldername)

# %% movie
dir_name = f'./{alpha}_{N}_{last_chunk}_{chunks_to_skip}'
if os.path.exists(dir_name):
    i = 1
    while os.path.exists(dir_name + f' ({i})'):
        i += 1
    dir_name = dir_name + f'_{i}'
os.mkdir(dir_name)
# %%
df0 = pd.DataFrame(np.array(motion_chunks[1][5:],dtype=float),columns=motion_chunks[0][4][2:])
# 50th row in df0
floor_height = df0.y.min()
lid_height = df0.y.max()
box_height = lid_height - floor_height

# %%
# indexing third to fifth column
centroids = df0.iloc[:-6,2:5].to_numpy()
quaternions = df0.iloc[:-6,8:12].to_numpy()
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
scene.render('ipython', width=800, height=600, antialiasing=0.0001)

    
# %%
for i,chunk in enumerate(motion_chunks):
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
