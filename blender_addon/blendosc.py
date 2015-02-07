bl_info = {
    "name": "BlendOsc Preferences",
    "author": "Hansi Raber",
    "version": (1, 0),
    "blender": (2, 65, 0),
    "location": "BlendOsc",
    "description": "BlendOsc Addon",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"}

import bpy
from bpy.props import StringProperty, IntProperty, BoolProperty
import socket
import sys
from bpy.app.handlers import persistent

sock = None

 
#panel with per object settings    
class myPanel(bpy.types.Panel):     # panel to display new property
    bl_space_type = "VIEW_3D"       # show up in: 3d-window
    bl_region_type = "UI"           # show up in: properties panel
    bl_label = "Oscilloscope"           # name of the new panel
 
    def draw(self, context):
        # display value of "foo", of the active object
        self.layout.prop(bpy.context.active_object, "freq")
        self.layout.prop(bpy.context.active_object, "tracefrom", slider=True)
        self.layout.prop(bpy.context.active_object, "tracerange",slider=True)
 
#panel with global settings and connect/disconnect buttons
class OBJECT_PT_pingpong(bpy.types.Panel):
    bl_label = "Oscilloscope"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"
 
    def draw_header(self, context):
        layout = self.layout
        layout.label(text="", icon="SPEAKER")
 
    def draw(self, context):
        global sock
        layout = self.layout
        layout.prop(bpy.context.window_manager, 'blendosc_maxvertices', text="Max Vertices")
        layout.prop(bpy.context.window_manager, 'blendosc_port', text="Port Number")
        layout.prop(bpy.context.window_manager, 'blendosc_sendpolys', text="Send data for edge removal")
        layout.prop(bpy.context.window_manager, 'blendosc_synthmode', text="Play all objects at once (add)")
        layout.prop(bpy.context.window_manager, 'blendosc_autosend')
        
        if sock is None:
            layout.operator("render.blendosc_connect", text="Connect")
        else:
            layout.operator("render.blendosc_disconnect", text="Disconnect")
 
# connect operation
class OBJECT_OT_pingpong_connect(bpy.types.Operator):
    bl_label = "Connect to BlendOsc"
   
    bl_idname = "render.blendosc_connect"
    bl_description = "Connect to BlendOsc"
 
    def execute(self, context):
        global sock
        TCP_IP = '127.0.0.1'
        TCP_PORT = bpy.context.window_manager.blendosc_port
        
        if sock is None: 
            try: 
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect((TCP_IP, TCP_PORT))
                sock.send( bytes("Z\n", 'UTF-8' ))
                self.report({'INFO'}, "Connected!")
                scene_updated(bpy.context.scene,True)
            except Exception as inst: 
                print( inst )
                print( type(inst) )
                print( inst.args )
                sock.close()
                sock = None
                print("nope")
                self.report({'ERROR'}, "Connection failed")
        else: 
            self.report({'INFO'}, "Was Already connected")
            
        return {'FINISHED'}

# disconnect operation
class OBJECT_OT_pingpong_disconnect(bpy.types.Operator): 
    bl_label = "Disconnect from BlendOsc"
    
    bl_idname="render.blendosc_disconnect"
    
    def execute(self, context): 
        global sock
        if sock is not None: 
            try: 
                sock.send( bytes("Z\n", 'UTF-8' ))
                sock.close()
                self.report({'INFO'}, "Connection closed")
            except:
                self.report({'ERROR'}, "Couldn't close connection")
                
            sock = None
        else: 
            self.report({'INFO'}, "Connection was already closed")
        
        return {'FINISHED'}




def world_to_camera_view(co):
    obj = bpy.context.scene.camera
    from mathutils import Vector   
    co_local = obj.matrix_world.normalized().inverted() * co
    z = -co_local.z

    scene = bpy.context.scene
    camera = obj.data
    frame = [-v for v in camera.view_frame(scene=scene)[:3]]
    if camera.type != 'ORTHO':
        if z == 0.0:
            return Vector((0.5, 0.5, 0.0))
        else:
            frame = [(v / (v.z / z)) for v in frame]

    min_x, max_x = frame[1].x, frame[2].x
    min_y, max_y = frame[0].y, frame[1].y

    x = (co_local.x - min_x) / (max_x - min_x)
    y = (co_local.y - min_y) / (max_y - min_y)

    return Vector((x, y, z))


def listObject(s,obj,max): 
    me = obj.to_mesh(scene=bpy.context.scene, apply_modifiers=True, settings='PREVIEW')
    count = len(me.vertices)
    if count >= max: 
        return ("", 0)
    
    msg = str(obj.freq) + ":" + str(obj.tracefrom) + ":" + str(obj.tracerange)
    msg = msg + ":" + listEdges(obj,me)
    if bpy.context.window_manager.blendosc_sendpolys: 
        msg = msg + ":" + listPolygons(obj,me)
    
    me.user_clear()
    bpy.data.meshes.remove(me)
    
    return (msg, count)
    
def listPolygons(obj,me): 
    msg = ""
    mat = obj.matrix_world

    first = True
    for vec in me.vertices:
        v = vec.co
        v = mat*v
        msg = msg + ("" if first else "&") + str(v[0])+" " + str(v[1]) + " " + str(v[2])
        first = False
        
    msg = msg + ":"
    
    first = True
    for poly in me.polygons: 
        p = ""
        for v in poly.vertices: 
            p = p + ("" if len(p)==0 else " ") + str(v)
        
        msg = msg + ("" if first else "&" ) + p
        first = False
    
    return msg
    

def listEdges(obj,me): 
    msg = ""
    vertices = me.vertices; 
    mat = obj.matrix_world
    #val = 0 if obj.active_shape_key == None else obj.active_shape_key.value
    
    first = True
    for vec in me.vertices:
        v = vec.co
        v = world_to_camera_view(mat*v)
        msg = msg + ("" if first else "&") + str(v[0])+" " + str(v[1])
        first = False
        
    msg = msg + ":"
    
    first = True
    for edge in me.edges:
        i1 = edge.vertices[0]
        i2 = edge.vertices[1]
        msg = msg + ("" if first else "&" ) + str(i1) + " " + str(i2)
        first = False
    
    return msg

@persistent
def scene_updated(scene,forceUpdate=False):
    global sock
    
    if sock is None: 
        return

    max = bpy.context.window_manager.blendosc_maxvertices
    
    if forceUpdate or bpy.data.objects.is_updated:
        msg = ""
        
        if bpy.context.window_manager.blendosc_synthmode: 
            msg = "add#"
        else: 
            msg = "serial#"
        
        for obj in bpy.context.selectable_objects: 
            if obj.type == "MESH": 
                (msg_,count_) = listObject(sock,obj,max)
                if count_ > 0: 
                    msg = msg + msg_ + "#"
                    max = max - count_
                if max <= 0: 
                    break

        if msg == "":
            msg = "Z"
            
        try: 
            sock.send( bytes(msg+"\n", 'UTF-8' ))
        except: 
            bpy.ops.render.blendosc_disconnect()
            
def forceUpdate(self, context):
    scene_updated(context.scene, True)

            
def initBlendosc():
    bpy.types.Object.freq = bpy.props.FloatProperty(
        name="Frequency", 
        default=50.0, 
        min=0.01, 
        max=15000.0
    )
    
    bpy.types.Object.tracefrom = bpy.props.FloatProperty(
        name="Trace from", 
        default=0, 
        min=0.00, 
        max=1.00
    )
    
    bpy.types.Object.tracerange = bpy.props.FloatProperty(
        name="Trace range", 
        default=1.0, 
        min=0.00, 
        max=1.0
    )
    

    
    bpy.types.WindowManager.blendosc_maxvertices = IntProperty(
            name="Maximum vertices per frame (higher means more cpu)",
            default=200,
            min=2
            )
            
    bpy.types.WindowManager.blendosc_synthmode = BoolProperty(
            name="Play all objects at once (ADD)",
            default=False, 
            update=forceUpdate
            )
            
    bpy.types.WindowManager.blendosc_port = IntProperty(
            name="Port Number",
            default=11995,
            min=1,
            max=65536
            )
    
    bpy.types.WindowManager.blendosc_autosend = BoolProperty(
            name="Automatically send when blend file changes",
            default=True,
            )

    bpy.types.WindowManager.blendosc_sendpolys = BoolProperty(
            name="Send data for hidden edge removal (Experimental)",
            default=False,
            update=forceUpdate
            )

    
    #bpy.app.handlers.scene_update_post.clear()
    #bpy.utils.register_class(myPanel)   # register panel

#@persistent
#def load_handler(somethingsomething):
#    print( "nothing" )
    

def register(): 
    bpy.app.handlers.scene_update_post.append(scene_updated)
    #bpy.app.handlers.load_post.append(load_handler)
    bpy.utils.register_module(__name__)
    
    initBlendosc()
    
def unregister():
   #bpy.utils.unregister_class(myPanel)
   bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
    register()
