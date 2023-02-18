# %%
import subprocess
# subprocess.call('dir', shell=True)
subprocess.run(["svn","--version"], shell=True)

from vapory import *

camera = Camera( 'location', [0,2,-3], 'look_at', [0,1,2] )
light = LightSource( [2,4,-3], 'color', [1,1,1] )
sphere = Sphere( [0,1,2], 2, Texture( Pigment( 'color', [1,0,1] )))

scene = Scene( camera, objects= [light, sphere])
scene.render('ipython',width=400, height=300)
# %%
