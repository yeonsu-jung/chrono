from direct.showbase.ShowBase import ShowBase
from panda3d.core import Geom, GeomNode, GeomTristrips, GeomVertexData, GeomVertexFormat, GeomVertexWriter
from panda3d.core import DirectionalLight, NodePath, PerspectiveLens
from panda3d.core import LVector3, Vec3, Vec4
import math

class MyApp(ShowBase):
    def create_cylinder(self, length, radius, sides=16):
        vertex_data = GeomVertexData('cylinder', GeomVertexFormat.getV3n3c4(), Geom.UHStatic)

        vertices = []
        vertex = GeomVertexWriter(vertex_data, 'vertex')
        normal = GeomVertexWriter(vertex_data, 'normal')
        color = GeomVertexWriter(vertex_data, 'color')

        # Create the vertices
        for i in range(sides):
            angle = i / float(sides) * 2.0 * math.pi
            x = radius * math.sin(angle)
            y = radius * math.cos(angle)
            z = 0.0
            vertices.append(Vec3(x, y, z))

            vertex.addData3f(x, y, z)
            normal.addData3f(x, y, z)
            color.addData4f(1.0, 1.0, 1.0, 1.0)

            vertex.addData3f(x, y, z+length)
            normal.addData3f(x, y, z+length)
            color.addData4f(1.0, 1.0, 1.0, 1.0)

        # Create the tristrips
        tristrips = GeomTristrips(Geom.UHStatic)

        for i in range(sides):
            tristrips.addVertex(i*2)
            tristrips.addVertex(i*2+1)

        tristrips.addVertex(0)
        tristrips.addVertex(1)

        tristrips.closePrimitive()

        cylinder_geom = Geom(vertex_data)
        cylinder_geom.addPrimitive(tristrips)

        return cylinder_geom

    def __init__(self):
        ShowBase.__init__(self)

        # Create a cylinder node path
        cylinder_geom_node = GeomNode('cylinder')
        cylinder_geom_node.addGeom(self.create_cylinder(2, 0.5))
        cylinder_node = self.render.attachNewNode(cylinder_geom_node)

        # Set the cylinder's position, rotation, and scale
        cylinder_node.setPos(0, 0, 0)
        cylinder_node.setHpr(0, 0, 0)
        cylinder_node.setScale(1, 1, 1)

        # Add lighting
        d_light = DirectionalLight('d_light')
        d_light.setColor(Vec4(1, 1, 1, 1))
        d_light_np = self.render.attachNewNode(d_light)
        d_light_np.setHpr(0, -60, 0)
        self.render.setLight(d_light_np)

        # Add a camera
        camera = self.camera
        lens = PerspectiveLens()
        lens.setFov(60)
        lens.setNear(0.1)
        lens.setFar(100)
        # camera.setLens(lens)
        camera.setPos(0, -5, 0)
        # Set up the background color
        self.setBackgroundColor(0.5, 0.5, 0.5, 1)

        # Start the main loop to render the scene
        self.run()

app = MyApp()




# from direct.showbase.ShowBase import ShowBase
# from panda3d.core import Geom, GeomNode, GeomTristrips, GeomVertexData, GeomVertexFormat, GeomVertexWriter
# from panda3d.core import NodePath
# from panda3d.core import LVector3, Vec3, Vec4
# import math

# class MyApp(ShowBase):
#     def create_cylinder(self, length, radius, sides=16):
#         vertex_data = GeomVertexData('cylinder', GeomVertexFormat.getV3n3c4(), Geom.UHStatic)

#         vertices = []
#         vertex = GeomVertexWriter(vertex_data, 'vertex')
#         normal = GeomVertexWriter(vertex_data, 'normal')
#         color = GeomVertexWriter(vertex_data, 'color')

#         # Create the vertices
#         for i in range(sides):
#             angle = i / float(sides) * 2.0 * math.pi
#             x = radius * math.sin(angle)
#             y = radius * math.cos(angle)
#             z = 0.0
#             vertices.append(Vec3(x, y, z))

#             vertex.addData3f(x, y, z)
#             normal.addData3f(x, y, z)
#             color.addData4f(1.0, 1.0, 1.0, 1.0)

#             vertex.addData3f(x, y, z+length)
#             normal.addData3f(x, y, z+length)
#             color.addData4f(1.0, 1.0, 1.0, 1.0)

#         # Create the tristrips
#         tristrips = GeomTristrips(Geom.UHStatic)

#         for i in range(sides):
#             tristrips.addVertex(i*2)
#             tristrips.addVertex(i*2+1)

#         tristrips.addVertex(0)
#         tristrips.addVertex(1)

#         tristrips.closePrimitive()

#         cylinder_geom = Geom(vertex_data)
#         cylinder_geom.addPrimitive(tristrips)

#         return cylinder_geom

#     def __init__(self):
#         ShowBase.__init__(self)

#         # Create a cylinder node path
#         cylinder_geom_node = GeomNode('cylinder')
#         cylinder_geom_node.addGeom(self.create_cylinder(20, 0.5))
#         cylinder_node = self.render.attachNewNode(cylinder_geom_node)

#         # Set the cylinder's position, rotation, and scale
#         cylinder_node.setPos(0, 0, 0)
#         cylinder_node.setHpr(0, 0, 0)
#         cylinder_node.setScale(10, 1, 1)

#         # Add the cylinder to the scene graph
#         cylinder_node.reparentTo(self.render)

#         # Start the main loop to render the scene
#         self.run()

# app = MyApp()


# from direct.showbase.ShowBase import ShowBase
# from panda3d.core import Geom, GeomNode, GeomTristrips, GeomVertexData, GeomVertexFormat, GeomVertexWriter
# from panda3d.core import NodePath
# from panda3d.core import LVector3, Vec3, Vec4
# import math

# class MyApp(ShowBase):
#     def create_cylinder(self, length, radius, sides=16):
#         vertex_data = GeomVertexData('cylinder', GeomVertexFormat.getV3n3c4(), Geom.UHStatic)

#         vertices = []
#         vertex = GeomVertexWriter(vertex_data, 'vertex')
#         normal = GeomVertexWriter(vertex_data, 'normal')
#         color = GeomVertexWriter(vertex_data, 'color')

#         # Create the vertices
#         for i in range(sides):
#             angle = i / float(sides) * 2.0 * math.pi
#             x = radius * math.sin(angle)
#             y = radius * math.cos(angle)
#             z = 0.0
#             vertices.append(Vec3(x, y, z))

#             vertex.addData3f(x, y, z)
#             normal.addData3f(x, y, z)
#             color.addData4f(1.0, 1.0, 1.0, 1.0)

#             vertex.addData3f(x, y, z+length)
#             normal.addData3f(x, y, z+length)
#             color.addData4f(1.0, 1.0, 1.0, 1.0)

#         # Create the tristrips
#         tristrips = GeomTristrips(Geom.UHStatic)

#         for i in range(sides):
#             tristrips.addVertex(i*2)
#             tristrips.addVertex(i*2+1)

#         tristrips.addVertex(0)
#         tristrips.addVertex(1)

#         tristrips.closePrimitive()

#         cylinder_geom = Geom(vertex_data)
#         cylinder_geom.addPrimitive(tristrips)

#         return cylinder_geom

#     def __init__(self):
#         ShowBase.__init__(self)

#         # Create a cylinder node path
#         cylinder_node = NodePath(GeomNode('cylinder'))
#         cylinder_node.attachNewNode(self.create_cylinder(2, 0.5))

#         # Set the cylinder's position, rotation, and scale
#         cylinder_node.setPos(0, 0, 0)
#         cylinder_node.setHpr(0, 0, 0)
#         cylinder_node.setScale(1, 1, 1)

#         # Add the cylinder to the scene graph
#         cylinder_node.reparentTo(self.render)

#         # Start the main loop to render the scene
#         self.run()

# app = MyApp()



# from direct.showbase.ShowBase import ShowBase
# from panda3d.core import *

# class MyApp(ShowBase):
#     def __init__(self):
#         ShowBase.__init__(self)

#         # Load the model
#         self.model = self.loader.loadModel("my_model.egg")

#         # Set the position of the model
#         self.model.setPos(0, 0, 0)

#         # Add the model to the scene graph
#         self.model.reparentTo(self.render)

#         # Add lighting to the scene
#         self.light = PointLight('light')
#         self.light.setColor(VBase4(1, 1, 1, 1))
#         self.light.setAttenuation((1, 0, 0.001))
#         self.light_np = self.render.attachNewNode(self.light)
#         self.light_np.setPos(0, 0, 50)
#         self.render.setLight(self.light_np)

# app = MyApp()
# app.run()


# import direct.directbase.DirectStart
# from panda3d.core import Geom, GeomNode, GeomTristrips, GeomVertexData, GeomVertexWriter
# from panda3d.core import LVector3, NodePath
# from panda3d.core import Vec3, Vec4
# from panda3d.core import TextureStage

# # Create a cylinder geometry
# def create_cylinder(length, radius, sides=16):
#     vertex_data = GeomVertexData('cylinder', GeomVertexFormat.getV3n3c4(), Geom.UHStatic)

#     vertices = []
#     vertex = GeomVertexWriter(vertex_data, 'vertex')
#     normal = GeomVertexWriter(vertex_data, 'normal')
#     color = GeomVertexWriter(vertex_data, 'color')

#     # Create the vertices
#     for i in range(sides):
#         angle = i / float(sides) * 2.0 * 3.14159
#         x = radius * math.sin(angle)
#         y = radius * math.cos(angle)
#         z = 0.0
#         vertices.append(Vec3(x, y, z))

#         vertex.addData3f(x, y, z)
#         normal.addData3f(x, y, z)
#         color.addData4f(1.0, 1.0, 1.0, 1.0)

#         vertex.addData3f(x, y, z+length)
#         normal.addData3f(x, y, z+length)
#         color.addData4f(1.0, 1.0, 1.0, 1.0)

#     # Create the tristrips
#     tristrips = GeomTristrips(Geom.UHStatic)

#     for i in range(sides):
#         tristrips.addVertex(i*2)
#         tristrips.addVertex(i*2+1)

#     tristrips.addVertex(0)
#     tristrips.addVertex(1)

#     tristrips.closePrimitive()

#     cylinder_geom = Geom(vertex_data)
#     cylinder_geom.addPrimitive(tristrips)

#     return cylinder_geom

# # Create a cylinder node path
# cylinder_node = NodePath(create_cylinder(2, 0.5))

# # Set the cylinder's position, rotation, and scale
# cylinder_node.setPos(0, 0, 0)
# cylinder_node.setHpr(0, 0, 0)
# cylinder_node.setScale(1, 1, 1)

# # Create a texture stage and load a texture
# ts = TextureStage('ts')
# ts.setMode(TextureStage.MModulate)
# tex = loader.loadTexture('my_texture.jpg')
# cylinder_node.setTexture(ts, tex)

# # Add the cylinder to the scene graph
# cylinder_node.reparentTo(render)

# # Start the main loop to render the scene
# run()
