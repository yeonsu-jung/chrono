# %%
# TO DO: write setup file
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
fname = foldername + '/sim_data.txt'
# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha200.0_RandomRods_Alpha200_N628_Date2023-02-14_23-23-50_tstep_1.00simtime_1.00/sim_data.txt'
num_atoms,num_time_steps,variables = check_dump_file(fname)
print(num_atoms,num_time_steps)

# %%
last_chunk = 1400
chunks_to_skip = 100
# %%
# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha200.0_RandomRods_Alpha200_N628_Date2023-02-14_23-23-50_tstep_1.00simtime_1.00/sim_data.txt'
import regex as re
# extract alpha and N from filename and store it to variables
_,alpha,N = re.match(r'.*alpha(\d+.\d+)_RandomRods_Alpha(\d+.\d+)_N(\d+)_Date.*',fname).groups()
rod_radius = 1
rod_length = float(alpha)*2

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
motion_chunks[1][:4]
# %%
contact_chunks[1]

# %%
inputs = read_inputs(foldername)
metadata = read_metadata(foldername)
# %%
column_names = contact_chunks[0][4].split()
column_names.append('rod1')
column_names.append('rod2')
# %%
# every df_rod
dfs_rod = []
for chunk in motion_chunks:
    print(chunk[1])
    rod_chunk = chunk[5:]
    dfs_rod.append(pd.DataFrame(np.array(rod_chunk,dtype=float),columns=rod_columns))
# %%
dfs_contact = []
for chunk in contact_chunks:
    print(chunk[1])
    start_point = int(chunk[3])
    data_fields = []
    for data_line in chunk[5:start_point-1]:
    # for data_line in chunk[5:]:
        data_fields.append(data_line.split())
    df = pd.DataFrame(np.array(data_fields,dtype=float),columns=column_names)

    dfs_contact.append(df)

# %%
new_dfs_contact = []
for df in dfs_contact:
    r1 = df['rod1'].values
    r2 = df['rod2'].values

    R = df.loc[:,['pc00','pc01','pc02','pc10','pc11','pc12','pc20','pc21','pc22']].values.reshape(-1,3,3)
    v = df.loc[:,['cfx','cfy','cfz']].values

    new_v = np.matmul(R,v[:,:,np.newaxis])[:,:,0]
    new_df = pd.DataFrame(np.hstack([df.loc[:,['rod1','rod2']],new_v]),columns=['rod1','rod2','cfx','cfy','cfz'])
    new_dfs_contact += [new_df]
    
# %%
ind = 2
def sum_over_duplicates(df):
    df = df.groupby(['rod1','rod2']).sum().reset_index()
    return df
summed = sum_over_duplicates(new_dfs_contact[ind])

which_rod = 100027
summed[summed['rod1'] == which_rod].loc[:,['cfx','cfy','cfz']].sum().to_numpy()*-1 + summed[summed['rod2'] == which_rod].loc[:,['cfx','cfy','cfz']].sum().to_numpy()
# %%
df_rod = dfs_rod[ind]
df_rod[df_rod['id']==which_rod].loc[:,'fx':'fz']

# %%










# %%
ind = 2
df_contact = dfs_contact[ind]
df_i = df_contact[df_contact['rod1']==which_rod]
df_i
# %%
df_j = df_contact[df_contact['rod2']==which_rod]
# df_j.loc[:,'cfx':'cfz'] = df_j.loc[:,'cfx':'cfz'].values * -1
df_j.loc[:,'cfx':'cfz'] = df_j.loc[:,'cfx':'cfz'].values * -1
df_c = pd.concat([df_i,df_j])

# %%
force_in_gc = []
for i,r in df_c.iterrows():
    rot_mat = np.array([r.pc00,r.pc01,r.pc02,r.pc10,r.pc11,r.pc12,r.pc20,r.pc21,r.pc22]).reshape(3,3)
    fvec = np.array([r.cfx,r.cfy,r.cfz])
    force_in_gc.append(np.matmul(rot_mat,fvec))

force_in_gc = np.array(force_in_gc)
np.sum(-force_in_gc,axis=0)
# %%



# %%
# sum over duplicates
ind = 2
def sum_over_duplicates(df):
    df = df.groupby(['rod1','rod2']).sum().reset_index()
    return df
summed = sum_over_duplicates(dfs_contact[ind])

summed[summed['rod1'] == which_rod].loc[:,['cfx','cfy','cfz']].sum().to_numpy()*-1 + summed[summed['rod2'] == which_rod].loc[:,['cfx','cfy','cfz']].sum().to_numpy()

# %%
# %%
from mpl_toolkits.mplot3d import Axes3D
%matplotlib qt

# Create a 3D quiver plot of the forces
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.quiver(pA[:,0], pA[:,1], pA[:,2], uf[:,0], uf[:,1], uf[:,2], length=1, normalize=True, color='red')
ax.quiver(pA0[:,0], pA0[:,1], pA0[:,2], uf0[:,0], uf0[:,1], uf0[:,2], length=1, normalize=True, color='blue')


# %%
print(pA)

# %%
print(uf0[i0].shape,uf[j0].shape)
# %%
np.hstack((uf0[i0],uf[j0]))

# %%
plt.plot(np.sum(uf0[i0]*uf[j0],axis=1))

# %%



# %%

a = np.array([2, 3])
b = np.array([[1, 5], [2, 3]])

# Check if all elements of a are equal to any row of b
np.any(b == a, axis=1)


# ismember = np.all(np.any(b == a, axis=1))

# %%
np.isin([100003,100270],uc,)


# %%
# unique element by row
unique_rows0 = np.unique(df0.iloc[:,-2:].to_numpy(),axis=0)
unique_rows = np.unique(df.iloc[:,-2:].to_numpy(),axis=0)

# print(df.iloc[:,-2:].to_numpy().shape,unique_rows.shape)
# %%
print(unique_rows0)

# index contacts
# get contact forces (sum?)
# track contact forces

contacts = df[['rod1', 'rod2']].to_numpy()
forces = df[['cfx', 'cfy', 'cfz']].to_numpy()

# Find unique contacts and sum the corresponding forces
uc = []
uf = []
for dfi in [df0,df]:
    unique_contacts, indices, counts = np.unique(contacts, return_index=True, return_counts=True, axis=0)
    force_in_gcs = np.zeros((len(unique_contacts), 3))
    for i, idx in enumerate(indices):
        force_in_gcs[i] = np.sum(forces[idx:idx+counts[i]], axis=0)
    uc.append(unique_contacts)
    uf.append(force_in_gcs)

# %%
# uc[0] is the current contacts
# uc[1] is the next contacts
# find indexes where uc[0] is in uc[1]
# Assume unique_contacts and force_in_gcs are already defined

# Find the indexes where uc[0] and uc[1] have the same row

corr = my_normalized_dot(uf[0],uf[1])

plt.plot(corr)

# %%

# %% main script
# fname = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt'


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











# %%
# %%
corrs = []
for chunk in contact_chunks[10:12]:
    data_fields = []
    for data_line in chunk[5:]:
        data_fields.append(data_line.split())
    df0 = df
    df = pd.DataFrame(np.array(data_fields,dtype=float),columns=column_names)
    uc0,uf0,pA0,pB0,cp0 = extract_unique_elements(df0)
    uc,uf,pA,pB,cp = extract_unique_elements(df)

    i0,j0 = ismember(uc0,uc)
    # corrs.append(np.sum(uf0[i0]*uf[j0],axis=1).sum()/np.sum(uf0[i0]*uf0[i0],axis=1).sum())
# %%
for chunk in motion_chunks[10:12]:
    data_fields = []
    for data_line in chunk[5:]:
        data_fields.append(data_line.split())