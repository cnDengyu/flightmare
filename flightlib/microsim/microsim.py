import os, platform
import yaml
from typing import List, Dict
import numpy as np
from collections import OrderedDict

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

class RigidBody:
    def __init__(self, name, position, quaternion, isreal=False):
        self.name = name
        self.position = position
        self.quaternion = quaternion
    
    def __init__(self, name, isreal=False):
        self.name = name
        self.position = [0, 0, 0]
        self.quaternion = [1, 0, 0, 0]
    
    def __init__(self, name, position, isreal=False):
        self.name = name
        self.position = position
        self.quaternion = [1, 0, 0, 0]

class MicroSim:
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
    
    def connect_unity(self):
        # Flightmare StartUp
        self.unity_cfg["env"]["num_envs"] = len(self.rigidbodies)
        # print(self.unity_cfg)
        self.unity = QuadrotorEnv_v1(yaml.dump(self.unity_cfg), False)
        self.unity_connected = self.unity.connectUnity()
        if not self.unity_connected:
            raise RuntimeError("Failed to connect to unity")

    def add_rigidbody(self, id : int, rb:RigidBody):
        if self.unity_connected:
            # print("Cannot add rigidbody after connected to unity")
            raise UserWarning("Cannot add rigidbody after connected to unity. Ignored.")
        else:
            self.rigidbodies[id] = rb

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
    
    def close_client(self):
        self.unity.disconnectUnity()
        if self._enable_natnet:
            self._nat.close()

    def natnet_read(self):
        if self._enable_natnet:
            rb_list = self._nat.get_rigidbodies()

    
    def sendState(self):
        state = np.zeros((len(self.rigidbodies),13), np.float32)
        for i, rb in self.rigidbodies.items():
            state[i, :3] = np.array(rb.position, np.float32)
            state[i, 3:7] = np.array(rb.quaternion, np.float32)
        self.unity.setState(state, self.frame)
        self.frame += 1

