# %%
import os
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from scipy.spatial.transform import Rotation
from vapory import *
from functions import *

import tkinter as tk
from tkinter import filedialog

# root.withdraw()

# %matplotlib qt
# %%
root = tk.Tk()
init_dir = change_path_across_platforms('C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/')
filename = filedialog.askopenfilename(initialdir = init_dir)
root.destroy()
foldername = os.path.dirname(filename)
# %%
motion_chunks = read_data(foldername + '/sim_data.txt',0,100,10)
metadata = read_metadata(foldername + '/metadata.txt')
rod_length = int(metadata['rod_length'])
rod_radius = int(metadata['rod_length'])

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
# %%


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
scene.render('test.png', width=800, height=600, antialiasing=0.0001)
