import os
import numpy as np
import pathlib as Path

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

def count_lines2(filename):
    return sum(1 for line in open(filename))


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

    total_num_rows = count_lines2(filename)

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
import itertools

def get_data_from_dump(filename,last_chunk,chunks_to_skip):
    row1, row2 = find_lines_between_two_flags(filename, 'ITEM: TIME', 'ITEM: TIME')
    num_rows_in_chunk = row2-row1
    
    slices = [(i*num_rows_in_chunk,i*num_rows_in_chunk+num_rows_in_chunk-1) for i in range(0,last_chunk,chunks_to_skip)]
    chunks = []    
    
    with open(filename) as f:
        for s1, s2 in slices:
            chunk_lines = itertools.islice(f, s1, s2)
            chunk = [line.split() for line in chunk_lines if not line.startswith('ITEM:')]
            chunks.append(chunk)
            print(len(chunks))
                
    return chunks
    
    # for (s1,s2) in slices:
    #     # print(all_lines[s1:s2])
    #     chunk = []        
    #     for line in read_lines_from_file2(filename, s1, s2):
    #         # if line.startswith('ITEM:'):
    #         #     continue
    #         chunk.append((line.split()))
            
    #     chunks.append(chunk)
    #     print(len(chunks))
    # return chunks

# %%
def read_inputs(foldername):
    with open(foldername + '/inputs.txt') as f:
        inputs = {}
        for line in f:
            key, value = line.split(' ')
            inputs[key] = value.strip()
    return inputs

# import yaml
# read yaml file
def read_metadata(filename):
    with open(filename) as f:
        inputs = {}
        for line in f:
            key, value = line.split(' ')
            inputs[key] = value.strip()
    return inputs


def find_dropbox_path():
    if os.name == 'nt':
        dropbox_path = 'C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims'
    elif os.name == 'posix':
        dropbox_path = '/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims'
    return dropbox_path

def change_path_across_platforms(path):    
    # if posix path
    if (os.path.sep == '/') & (os.name == 'nt'):
        path = path.replace('/Users/', 'C:/Users/')
        path = path.replace('yeonsu', 'yjung')        
    elif (os.path.sep == '\\') & (os.name == 'posix'):
        path = path.replace('\\', '/')
        path = path.replace('yjung', 'yeonsu')
        path = path.replace('C:\\','/')
    return path



def read_data(filename,start_chunk,last_chunk,chunks_to_skip):    
    chunks_to_read = range(start_chunk,last_chunk,chunks_to_skip)
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
                    print(f"{len(chunks)}",end='\r')
                if count in chunks_to_read:
                    chunk.append(line.strip('\n').split())
            elif count in chunks_to_read:
                chunk.append(line.strip('\n').split())
            if count == last_chunk:
                break                
    return chunks

# should deprecate these?
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

def split_chunks(chunks):
    column_names = chunks[0][0].split()[2:]
    timesteps = [chunk[0].split()[0] for chunk in chunks]
    num_atoms = [chunk[0].split()[1] for chunk in chunks]
    data = [chunk[1:] for chunk in chunks]
    return data, column_names, timesteps, num_atoms

def split_rows(chunks):
    chunk2 = []
    for chunk in chunks:
        chunk2.append([row.split() for row in chunk])
        print(f"{len(chunk2)}",end='\r')
    return chunk2