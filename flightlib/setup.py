import os
import glob
import shutil
import re
import sys
import platform
import subprocess

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        '''
        FLIGHTLIB_EXTERNAL_FILES = os.environ["FLIGHTMARE_PATH"] + \
            "/flightlib/externals/"
        # --------------------------------
        # remove cached external files
        # a hack to solve some cmake error when using "pip install ."
        try:
            for i, p in enumerate(glob.glob(os.path.join(FLIGHTLIB_EXTERNAL_FILES, "*"))):
                shutil.rmtree(p)
                print("Removing some cache file: ", p)
        except:
            pass
        FLIGHTLIB_BUILD_FILES = os.environ["FLIGHTMARE_PATH"] + \
            "/flightlib/build/"
        # --------------------------------
        # remove cached files
        # a hack to solve some cmake error when using "pip install ."
        try:
            for i, p in enumerate(glob.glob(os.path.join(FLIGHTLIB_BUILD_FILES, "*"))):
                shutil.rmtree(p)
                print("Removing some cache file: ", p)
        except:
            pass
        # --------------------------------
        '''
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(
                re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    debug = True

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(
            self.get_ext_fullpath(ext.name)))
        # required for auto-detection of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        # cfg = 'Debug' if self.debug else 'Release'
        cfg = 'Debug'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += [
                '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j4']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] +
                              cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] +
                              build_args, cwd=self.build_temp)


setup(
    name='flightgym',
    version='0.0.1',
    author='Yunlong Song',
    author_email='song@ifi.uzh.ch',
    description='Flightmare: A Quadrotor Simulator.',
    long_description='',
    ext_modules=[CMakeExtension('flightlib')],
    install_requires=['gym==0.11', 'ruamel.yaml',
                      'numpy', 'stable_baselines==2.10.1'],
    cmdclass=dict(build_ext=CMakeBuild),
    include_package_data=True,
    zip_safe=False,
)
