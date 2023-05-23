import os, platform
import yaml
from typing import List, Dict
import numpy as np
from collections import OrderedDict

'''
-------------------Importer---------------------

For Windows, 
    the microsim package need two environment variables.
    OpenCV_DIR      is the path of OpenCV build folder. The aimed dll is opencv_world470.dll
    FLIGHTMARE_PATH is the source code folder of flightmare. The aimed dll is NatNetLib.dll

For Linux, 
    users need to add opencv_world470.so and libNatNet.so to /usr/local/bin manually.
    developers need to install OpenCV 4.7.0 as a cmake package.

'''
plat = platform.system()
if plat == "Windows":
    # in Python 3.8+, dll can't be found in sys.path, need to add it explicitly
    # for convenient, it is recommanded to drop opencv_world470.dll to the same directory with python.exe
    opencv_dll_path = os.path.join(os.environ["OpenCV_DIR"], "x64\\vc16\\bin")
    os.add_dll_directory(opencv_dll_path)
    print("Added dll path:" + opencv_dll_path + " for OpenCV")
    natnet_dll_path = os.path.join(os.environ["FLIGHTMARE_PATH"], "flightlib\\externals\\natnet\\lib\\x64")
    os.add_dll_directory(natnet_dll_path)
    print("Added dll path:" + natnet_dll_path + " for NatNet SDK")
else:
    pass

from flightgym import NatNetManager, QuadrotorEnv_v1

'''

------------------ Communication Interface ----------------------

The RigidBody class is the communication interface of MicroSim.

parameters:
    name: str               the name of a rigidbody, such as "cf4"
    position: List[float]   the 3d position of a rigidbody
    quaternion: List[float] the quaternion of a rigidbody
    isreal: bool            if the rigidbody info is readed from motion capture system, it is true.

'''

class RigidBody:
    def __init__(self, name: str = "NoName", 
                 position: List[float] = [0,0,0], 
                 quaternion: List[float] = [1,0,0,0], 
                 isreal: bool = False):
        self.name = name
        self.position = position
        self.quaternion = quaternion

'''

---------------- MicroSim Wrapper --------------------

The Wrapper that unions sim and real.

The recommanded usage is:

    Create client -> Add rigid body -> Connect unity -> Read mocap data -> Send State -> Close client

'''

class MicroSim:
    # Create client
    def __init__(self, enable_natnet=False):
        self._enable_natnet = enable_natnet
        self.rigidbodies : OrderedDict[int, RigidBody] = {}

        # NatNet StartUp
        if enable_natnet:
            self._nat = NatNetManager()
            if not 0 == self._nat.connect():
                raise RuntimeError("Failed to connect to Motive")
            self._nat.get_description()
            rb_list = self._nat.get_rigidbodies()
            for rb in rb_list:
                self.rigidbodies[rb['id']] = RigidBody(rb['name'], rb['position'], rb['quaternion'], isreal=True)
        
        self.unity = None
        self.frame = 0
        self.unity_connected = False
        self.unity_cfg = {}
        self.unity_cfg["env"] = {}
        self.unity_cfg["env"]["seed"] = 1
        self.unity_cfg["env"]["scene_id"] = 0
        self.unity_cfg["env"]["render"] = "yes"
        self.unity_cfg["env"]["num_threads"] = 1
    
    # Add rigid body
    def add_rigidbody(self, id : int, rb:RigidBody):
        if self.unity_connected:
            # print("Cannot add rigidbody after connected to unity")
            raise UserWarning("Cannot add rigidbody after connected to unity. Ignored.")
        else:
            self.rigidbodies[id] = rb
    
    # Connect unity
    def connect_unity(self):
        # Flightmare StartUp
        self.unity_cfg["env"]["num_envs"] = len(self.rigidbodies)
        print("Total number of rigidbodies:" + str(len(self.rigidbodies)))
        self.unity = QuadrotorEnv_v1(yaml.dump(self.unity_cfg), False)
        self.unity_connected = self.unity.connectUnity()
        if not self.unity_connected:
            raise RuntimeError("Failed to connect to unity")
    
    # Read mocap data
    def natnet_read(self):
        if self._enable_natnet:
            rb_list = self._nat.get_rigidbodies()
            for rb in rb_list:
                self.rigidbodies[rb['id']].position = rb['position']
                self.rigidbodies[rb['id']].quaternion = rb['quaternion']

    # Send State
    def send_state(self):
        state = np.zeros((len(self.rigidbodies),13), np.float32)
        index = 0
        for i, rb in self.rigidbodies.items():
            state[index, :3] = np.array(rb.position, np.float32)
            state[index, 3:7] = np.array(rb.quaternion, np.float32)
            index += 1
        self.unity.setState(state, self.frame)
        self.frame += 1

    def reset_client(self):
        self.rigidbodies.clear()

        if self._enable_natnet:
            self._nat.reset()
            self._nat.get_description()
            rb_list = self._nat.get_rigidbodies()
            for rb in rb_list:
                self.rigidbodies[rb['id']] = RigidBody(rb['name'], rb['position'], rb['quaternion'], isreal=True)

        self.unity.disconnectUnity()
        connected = self.unity.connectUnity()
        if not connected:
            raise RuntimeError("Failed to connect to unity")
    
    # Close client
    def close_client(self):
        self.unity.disconnectUnity()
        if self._enable_natnet:
            self._nat.close()

