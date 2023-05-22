import os, platform
from ruamel.yaml import YAML, dump, RoundTripDumper
from typing import List
import numpy as np

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
    def __init__(self, id, name, position, quaternion, isreal=False):
        self.id = id
        self.name = name
        self.position = position
        self.quaternion = quaternion
    
    def __init__(self, id, name, isreal=False):
        self.id = id
        self.name = name
        self.position = [0, 0, 0]
        self.quaternion = [1, 0, 0, 0]

class MicroSim:
    def __init__(self, enable_natnet=False):
        self.enable_natnet = enable_natnet
        self.rigidbodies : List[RigidBody] = []

        # NatNet StartUp
        if enable_natnet:
            self.nat = NatNetManager()
            self.nat.connect()
            self.nat.get_description()
            rb_list = self.nat.get_rigidbodies()
            for rb in rb_list:
                self.rigidbodies.append(RigidBody(rb['id'], rb['name'], rb['position'], rb['quaternion'], isreal=True))
        
        self.unity = None
        self.frame = 0
        self.unity_connected = False
    
    def connectUnity(self):
        # Flightmare StartUp
        cfg = YAML().load(open(os.environ["FLIGHTMARE_PATH"] +
                           "/flightlib/configs/vec_env.yaml", 'r'))
        cfg["env"]["render"] = "yes"
        cfg["env"]["num_envs"] = len(self.rigidbodies)
        self.unity = QuadrotorEnv_v1(dump(cfg, Dumper=RoundTripDumper), False)
        self.unity.connectUnity()
        self.unity_connected = True

    def add_rigidbody(self, rb:RigidBody):
        if self.unity_connected:
            print("Cannot add rigidbody after connected to unity")
        else:
            self.rigidbodies.append(rb)

    def resetClient(self):
        self.rigidbodies.clear()

        if self.enable_natnet:
            self.nat.reset()
            self.nat.get_description()
            rb_list = self.nat.get_rigidbodies()
            for rb in rb_list:
                self.rigidbodies.append(RigidBody(rb['id'], rb['name'], rb['position'], rb['quaternion'], isreal=True))

        self.unity.disconnectUnity()
        self.unity.connectUnity()
    
    def closeClient(self):
        self.unity.disconnectUnity()
        self.nat.close()
    
    def sendState(self):
        state = np.zeros((len(self.rigidbodies),13), np.float32)
        for i, rb in enumerate(self.rigidbodies):
            state[i, :3] = np.array(rb.position, np.float32)
            state[i, 3:7] = np.array(rb.quaternion, np.float32)
        self.unity.setState(state, self.frame)
        self.frame += 1

