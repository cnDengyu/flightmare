import os, platform

plat = platform.system()
if plat == "Windows":
    # in Python 3.8+, dll can't be found in sys.path, need to add it explicitly
    # for convenient, it is recommanded to drop opencv_world470.dll to the same directory with python.exe
    opencv_dll_path = os.path.join(os.environ["OpenCV_DIR"], "x64\\vc16\\bin")
    os.add_dll_directory(opencv_dll_path)
else:
    pass

from flightgym import *
