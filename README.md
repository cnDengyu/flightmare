# MicroSim

![License](https://img.shields.io/badge/License-MIT-blue.svg) 

**MicroSim** is a flexible modular swarm simulator modified from [Flightmare](https://github.com/uzh-rpg/flightmare)

The MicroSim is under development.

## Install from source

First, install opencv and cmake.

For Ubuntu, run the apt command (or build and install them from source if you like).
```
sudo apt install libopencv-dev build-essential cmake git
```
For Windows, please 
1. Download and install Visual Studio or other compiler
2. Download and install CMake
3. Download OpenCV and set environment variable `OpenCV_DIR` to the `build` folder of OpenCV.

Next, clone the source code
```
git clone https://github.com/cnDengyu/flightmare
```
Set environment variable `FLIGHTMARE_PATH` to the flightmare folder.

Then, go to flightlib and install
```
cd flightlib
pip install . --verbose
```

## Updates
2023.05.24 Release 0.1.2, add NatNet to get info from motion capture system.

2023.05.19 Release 0.1.1, support cross-platform build.

2023.01.07 Pre-release 0.1.0, a single Windows version.

## Publication

If you use this code in a publication, please cite the following paper **[PDF](http://rpg.ifi.uzh.ch/docs/CoRL20_Yunlong.pdf)** of [Flightmare](https://github.com/uzh-rpg/flightmare) 

```
@inproceedings{song2020flightmare,
    title={Flightmare: A Flexible Quadrotor Simulator},
    author={Song, Yunlong and Naji, Selim and Kaufmann, Elia and Loquercio, Antonio and Scaramuzza, Davide},
    booktitle={Conference on Robot Learning},
    year={2020}
}
```

## License
This project is released under the MIT License. Please review the [License file](LICENSE) for more details.
