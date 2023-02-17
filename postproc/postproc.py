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

def rearrange(data):
    num_columns = np.min(data.shape)

    # N = int(np.max(data.shape)/num_atoms)
    # num_time_steps = len(timesteps)
    # assert(N == num_time_steps)


class postproc:
    def __init__(self, data, column_names, timesteps, num_atoms):
        self.data = data
        self.column_names = column_names
        self.timesteps = timesteps
        self.num_atoms = num_atoms
    

    # %%
# %%
# Define a function to parse a LAMMPS plain-text dump file into a NumPy array
def parse_lammps_dump_file(filename, rows_to_read=100):
    # Read the file and extract the header and data lines
    # with open(filename) as f:
    #     lines = f.readlines()

    lines = []
    with open(filename) as f:
        for i in range(rows_to_read):
            lines.append(f.readline())

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

    # arr = np.array(data)

    return data, column_names, timesteps, num_atoms

# %% main
import os

# if system is windows
if os.name == 'nt':
    data, column_names, timesteps, num_atoms = parse_lammps_dump_file('C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt')
elif os.name == 'posix':
    data, column_names, timesteps, num_atoms = parse_lammps_dump_file('/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt', rows_to_read=300000)

df = np.array(data)
df[:,2:5]
df[:,8:12]

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

quaternions = df[:,8:12]

# Example y-axis vector
y_axis = (0.0, 1.0, 0.0)

# Transform the y-axis vector using each quaternion in the array
orientations = []
for q in quaternions:
    rotated_y_axis = quaternion_vector_rotation(q, y_axis)
    orientations.append(np.array(rotated_y_axis))

rod_length = 38*2
rod_radius = 1

centroids = df[:,2:5]
lines = []
for i, c in enumerate(centroids):
    lines.append( np.hstack((c + orientations[i]*rod_length/2,c - orientations[i]*rod_length/2)))

print(lines)

# %%
from vapory import *

camera = Camera( 'location', [0,5,-200], 'look_at', [0,5,2] )
light = LightSource( [2,4,-3], 'color', [1,1,1] )
cylinder = Cylinder( [0,1,2], [0,10,2], 2, Texture( Pigment( 'color', [1,0,1] )))

# """Cylinder(
#            [Base_Point], [Cap_Point], Radius
#            *[ 'open', ]*[OBJECT_MODIFIERS...]
#            )"""

scene = Scene( camera, objects= [light, cylinder])
scene.render("purple_sphere.png", width=400, height=300)


# %%
camera = Camera( 'location', np.mean(centroids,axis=0)+[0,0,-300], 'look_at', np.mean(centroids,axis=0))
light = LightSource( np.mean(centroids,axis=0) + [200,400,-500], 'color', [1.2,1.2,1.2] )
bgd = Background( "color", [1,1,1] )
# sun = LightSource([1500,2500,-2500], 'color',1)

# sky = Sphere(  [0,0,0],1, 'hollow',
#               Texture(  Pigment(  'gradient', [0,1,0],
#                                   ColorMap([0, 'color', 'White'],
#                                            [1, 'color', 'Blue' ]),
#                                   'quick_color', 'White'),
#                         Finish( 'ambient', 1, 'diffuse', 0)),
#               'scale', 10000)


cylinders = []
for line in lines:
    xx = [line[0],line[3]]
    yy = [line[1],line[4]]
    zz = [line[2],line[5]]
    base = [xx[0],yy[0],zz[0]]
    cap = [xx[1],yy[1],zz[1]]
    cylinders.append(Cylinder( base, cap, 1, Texture( Pigment( 'color', [1,0,1] ))))
    # cylinders.append(Cylinder( base, cap, 1, Pigment('color', [0, 0, 1]),Finish('phong', 0.8,'reflection', 0.5)))
    

scene = Scene( camera, objects= [bgd,light,*cylinders])
scene.render("ipython", width=800, height=600)

# %%
def scene(t):
    """ Returns the scene at time 't' (in seconds) """
    starting_point = int(t)*num_atoms
    ending_point = starting_point+num_atoms    
    tmp = lines[starting_point:ending_point]
    cylinders = []
    for line in tmp:
        xx = [line[0],line[3]]
        yy = [line[1],line[4]]
        zz = [line[2],line[5]]
        base = [xx[0],yy[0],zz[0]]
        cap = [xx[1],yy[1],zz[1]]
        cylinders.append(Cylinder( base, cap, 1, Texture( Pigment( 'color', [1,0,1] ))))
    
    return Scene( camera, objects= [bgd,light,*cylinders])
def make_frame(t,nm):
    return scene(t).render(nm,width=800, height=600, antialiasing=0.001)

# %%
for i in range(0,1200,2):
    make_frame(i,f"example2/img_{i}.png")
# %%
from moviepy.editor import VideoClip

import os
# os.environ["IMAGEIO_FFMPEG_EXE"] = "/Users/yeonsu/opt/anaconda3/envs/vis-space/bin/ffmpeg"

VideoClip(make_frame, duration=4).write_videofile("anim.mp4",fps=1)

# %%
os.environ["IMAGEIO_FFMPEG_EXE"] = "/Users/yeonsu/opt/anaconda3/envs/vis-space/bin/ffmpeg"
os.environ["IMAGEIO_FFMPEG_EXE"]

# # %%
# # TO DO: write setup file

# import numpy as np
# from matplotlib import pyplot as plt
# from scipy.spatial.transform import Rotation
# %matplotlib qt

# # %%
# def my_float(field):
#     if 'nan' in field:
#         field = np.NaN
#     return float(field)

# def rearrange(data):
#     num_columns = np.min(data.shape)

#     # N = int(np.max(data.shape)/num_atoms)
#     # num_time_steps = len(timesteps)
#     # assert(N == num_time_steps)


# class postproc:
#     def __init__(self, data, column_names, timesteps, num_atoms):
#         self.data = data
#         self.column_names = column_names
#         self.timesteps = timesteps
#         self.num_atoms = num_atoms
    

#     # %%
# # %%
# # Define a function to parse a LAMMPS plain-text dump file into a NumPy array
# def parse_lammps_dump_file(filename, rows_to_read=100):
#     # Read the file and extract the header and data lines
#     # with open(filename) as f:
#     #     lines = f.readlines()

#     lines = []
#     with open(filename) as f:
#         for i in range(rows_to_read):
#             lines.append(f.readline())

#     header_lines = [line for line in lines if line.startswith('ITEM: ATOMS')]
#     header_positions = [i for i,line in enumerate(lines) if line.startswith('ITEM: ATOMS')]
#     last_header_position = header_positions[-1]

#     num_atoms = int([lines[i+1] for i,line in enumerate(lines) if line.startswith('ITEM: NUMBER')][0].split()[0])

#     if (len(lines) - last_header_position != num_atoms + 1):
#         print("Last timestep is incomplete")

#     data_lines = [line for line in lines if (not line.startswith('ITEM:')) and line.find(' ') != -1]
#     timesteps = [float(lines[i+1].split()[0]) for i,line in enumerate(lines) if line.startswith('ITEM: TIME')]

#     # Extract the column names from the last header line
#     header_line = header_lines[-1].split()[2:]
#     column_names = ['id', 'type'] + header_line

#     # Extract the timestep value from the "ITEM: TIMESTEP" header line
#     # timestep_line = [line for line in lines if line.startswith('ITEM: TIMESTEP')]
#     # print(timestep_line)
#     # timestep = [line for line in lines if (not line.startswith('ITEM:')) and (line.find(' ') == -1)]    

#     # Parse the data lines into a NumPy array
#     data = []
#     for line in data_lines:
#         fields = line.split()
#         data.append([my_float(field) for field in fields])

#     # arr = np.array(data)

#     return data, column_names, timesteps, num_atoms

# # %% main
# import os

# # if system is windows
# if os.name == 'nt':
#     data, column_names, timesteps, num_atoms = parse_lammps_dump_file('C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt')
# elif os.name == 'posix':
#     data, column_names, timesteps, num_atoms = parse_lammps_dump_file('/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/alpha76.0_RandomRods_Alpha76_N238_Date2023-02-14_23-24-30_tstep_0.50simtime_0.50/sim_data.txt', rows_to_read=300000)

# # %%
# df = np.array(data)
# df[:,2:5]
# df[:,8:12]

# def quaternion_multiply(q1, q2):
#     w1, x1, y1, z1 = q1
#     w2, x2, y2, z2 = q2
#     w = w1*w2 - x1*x2 - y1*y2 - z1*z2
#     x = w1*x2 + x1*w2 + y1*z2 - z1*y2
#     y = w1*y2 - x1*z2 + y1*w2 + z1*x2
#     z = w1*z2 + x1*y2 - y1*x2 + z1*w2
#     return (w, x, y, z)

# def quaternion_conjugate(q):
#     w, x, y, z = q
#     return (w, -x, -y, -z)

# def quaternion_vector_rotation(q, v):
#     qv = (0.0, *v)
#     return quaternion_multiply(quaternion_multiply(q, qv), quaternion_conjugate(q))[1:]
#     # return quaternion_multiply(quaternion_multiply(q, qv), q)[1:]

# # quaternions = [
# #     (0.707, 0.0, 0.0, 0.707),  # Example quaternion for a 90-degree rotation around the z-axis
# #     (0.5, 0.5, 0.5, 0.5),  # Example quaternion for a 120-degree rotation around an axis in the x-y plane
# #     (0.866, 0.0, 0.5, 0.0)  # Example quaternion for a 60-degree rotation around an axis in the x-z plane
# # ]

# quaternions = df[:,8:12]

# # Example y-axis vector
# y_axis = (0.0, 1.0, 0.0)

# # Transform the y-axis vector using each quaternion in the array
# orientations = []
# for q in quaternions:
#     rotated_y_axis = quaternion_vector_rotation(q, y_axis)
#     orientations.append(np.array(rotated_y_axis))
# # %%
# rod_length = 38*2
# rod_radius = 1

# centroids = df[:,2:5]
# lines = []
# for i, c in enumerate(centroids):
#     lines.append( np.hstack((c + orientations[i]*rod_length/2,c - orientations[i]*rod_length/2)))

# print(lines)

# # %%
# from vapory import *

# camera = Camera( 'location', [0,5,-200], 'look_at', [0,5,2] )
# light = LightSource( [2,4,-3], 'color', [1,1,1] )
# cylinder = Cylinder( [0,1,2], [0,10,2], 2, Texture( Pigment( 'color', [1,0,1] )))

# # """Cylinder(
# #            [Base_Point], [Cap_Point], Radius
# #            *[ 'open', ]*[OBJECT_MODIFIERS...]
# #            )"""

# scene = Scene( camera, objects= [light, cylinder])
# scene.render("purple_sphere.png", width=400, height=300)


# # %%
# camera = Camera( 'location', np.mean(centroids,axis=0)+[0,0,-300], 'look_at', np.mean(centroids,axis=0))
# light = LightSource( np.mean(centroids,axis=0) + [200,400,-500], 'color', [1.2,1.2,1.2] )
# bgd = Background( "color", [1,1,1] )
# # sun = LightSource([1500,2500,-2500], 'color',1)

# # sky = Sphere(  [0,0,0],1, 'hollow',
# #               Texture(  Pigment(  'gradient', [0,1,0],
# #                                   ColorMap([0, 'color', 'White'],
# #                                            [1, 'color', 'Blue' ]),
# #                                   'quick_color', 'White'),
# #                         Finish( 'ambient', 1, 'diffuse', 0)),
# #               'scale', 10000)


# cylinders = []
# for line in lines:
#     xx = [line[0],line[3]]
#     yy = [line[1],line[4]]
#     zz = [line[2],line[5]]
#     base = [xx[0],yy[0],zz[0]]
#     cap = [xx[1],yy[1],zz[1]]
#     cylinders.append(Cylinder( base, cap, 1, Texture( Pigment( 'color', [1,0,1] ))))
#     # cylinders.append(Cylinder( base, cap, 1, Pigment('color', [0, 0, 1]),Finish('phong', 0.8,'reflection', 0.5)))
    

# scene = Scene( camera, objects= [bgd,light,*cylinders])
# scene.render("ipython", width=800, height=600)

# # %%
# def scene(t):
#     """ Returns the scene at time 't' (in seconds) """
#     starting_point = int(t)*num_atoms
#     ending_point = starting_point+num_atoms    
#     tmp = lines[starting_point:ending_point]
#     cylinders = []
#     for line in tmp:
#         xx = [line[0],line[3]]
#         yy = [line[1],line[4]]
#         zz = [line[2],line[5]]
#         base = [xx[0],yy[0],zz[0]]
#         cap = [xx[1],yy[1],zz[1]]
#         cylinders.append(Cylinder( base, cap, 1, Texture( Pigment( 'color', [1,0,1] ))))
    
#     return Scene( camera, objects= [bgd,light,*cylinders])
# def make_frame(t,nm):
#     return scene(t).render(nm,width=800, height=600, antialiasing=0.001)

# # %%
# for i in range(300):
#     make_frame(i,f"example/img_{i}.png")
# # %%
# from moviepy.editor import VideoClip

# import os
# # os.environ["IMAGEIO_FFMPEG_EXE"] = "/Users/yeonsu/opt/anaconda3/envs/vis-space/bin/ffmpeg"

# VideoClip(make_frame, duration=4).write_videofile("anim.mp4",fps=1)

# # %%
# os.environ["IMAGEIO_FFMPEG_EXE"] = "/Users/yeonsu/opt/anaconda3/envs/vis-space/bin/ffmpeg"
# os.environ["IMAGEIO_FFMPEG_EXE"]