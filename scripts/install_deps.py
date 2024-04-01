import os
import sys
import functools
import enum
import subprocess

from common import Cmd, printColored, bcolors

GITHUB_URL = 'https://github.com/'
GIT_END = '.git'

"""
    How to add new dep ?
    1. Add Dep to LibType
    2. In Consts update getURL to be able download by git
    3. nixLibName add library. If Empty will be downloaded by git url. Need for packet manager
    4. Add lib cmake options libCmakeOptions if no make empty string
"""



class PlatformType(str, enum.Enum):
    Windows = 'Windows'
    Linux = 'Linux'
    MacOS = 'Mac'


def get_platform():
    platforms = {
        'linux': 'Linux',
        'linux1': 'Linux',
        'linux2': 'Linux',
        'darwin': 'Mac',
        'win32': 'Windows'
    }
    if sys.platform not in platforms:
        return sys.platform

    return PlatformType(platforms[sys.platform])


# new libs add here
class LibType(str, enum.Enum):
    GLFW = 'glfw'
    SPDLOG = 'spdlog'
    #FREETYPE = 'freetype'

    @classmethod
    def libs(cls) -> list:
        return list(cls.__members__.values())


class Consts:
    @staticmethod
    def getURL(lib: LibType) -> str:
        githubUserNames = {
            'glfw': 'glfw',
            'spdlog': 'gabime'
        }
        return f'{GITHUB_URL}{githubUserNames[lib.value]}/{lib.value}{GIT_END}'


class WinCompiler(enum.IntEnum):
    No = 0
    MinGW = 1
    VS = 2

    def __str__(self):
        if self.value == self.MinGW:
            return "MinGW"
        if self.value == self.VS:
            return "Visual Studio"

class WinVSVersion(enum.IntEnum):
    No = 0
    VS_19 = 1
    VS_22 = 2

    def __str__(self):
        if self.value == self.VS_19:
            return "Visual Studio 16 2019"
        if self.value == self.VS_22:
            return "Visual Studio 17 2022"


class NixLibName:
    MacOSName: str
    LinuxName: str

    def __init__(self, mac, linux):
        self.MacOSName = mac
        self.LinuxName = linux


class CMakeOptions:
    generator: str
    os_make: str
    options: str

    def __init__(self, generator, options, os_make=''):
        self.generator = generator
        self.options = options
        self.os_make = os_make


class BuildMode(enum.IntEnum):
    Debug = 1
    Release = 2

    def __str__(self):
        if self.value == self.Debug:
            return "Debug"
        if self.value == self.Release:
            return "Release"


class Lib:
    def __init__(self, name: str, giturl: str,
                 macOSname: str, linuxName: str,
                 cmakeOptions: CMakeOptions, 
                 configuration: str = "Debug"
                 ):
        self.name = name
        self.giturl = giturl
        self.macOSname = macOSname
        self.linuxName = linuxName
        self.cmakeOptions = cmakeOptions
        self.configuration = configuration
        self.cmds = []

    def __build_library(self, generator, os_make='', cmake_options=''):
        cmake_cmd = Cmd(f'cd {self.name} && mkdir build '
                           f'&& cd build && cmake .. -G "{generator}" -DCMAKE_BUILD_TYPE=Release {cmake_options} && {os_make}make '
                           f'&& sudo {os_make}make install')
        self.cmds.append(cmake_cmd)

    def __build_library_vs(self, generator, cmake_options=''):
        cmake_cmd = Cmd(f'cd {self.name} &&'
                           f'cmake -B build/ -G "{generator}" -A x64 -DCMAKE_CONFIGURATION_TYPES:STRING="{self.configuration}" '
                           f' -DCMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD=ON '
                           f'{cmake_options} && cmake --build build/ --config {self.configuration}'
                           f' && cmake --install build/ --config {self.configuration}')
        self.cmds.append(cmake_cmd)

    def __pack_cmds(self):
        currentPlatform = get_platform()

        if currentPlatform != PlatformType.Windows \
                and (self.macOSname != '' and self.linuxName != ''):
            packman_cmd = ''
            libname = ''
            if currentPlatform is PlatformType.MacOS:
                packman_cmd = 'brew install'
                libname = self.macOSname
            if currentPlatform is PlatformType.Linux:
                # TODO(a.raag): Support not only Ubuntu
                packman_cmd = 'sudo apt-get install -y'
                libname = self.linuxName

            self.cmds.append(Cmd(f'{packman_cmd} {libname}'))
        else:
            git_cmd = f'git clone {self.giturl}'
            self.cmds.append(Cmd(git_cmd))
            self.__build_library_vs(self.cmakeOptions.generator,
                                      self.cmakeOptions.options)                          
                                      
            #self.__build_library(self.cmakeOptions.generator,
            #                     self.cmakeOptions.os_make,
            #                     self.cmakeOptions.options)

    def run(self) -> bool:
        self.__pack_cmds()

        for cmd in self.cmds:
            printColored(bcolors.WARNING, f'Execute command {cmd.getCmd()}')
            try:
                proc = subprocess.Popen(cmd.getCmd(), shell=True)
                proc.wait()
            except OSError:
                return False
        return True
        
    def __str__(self):
        return str(self.__dict__)



class DepsCache:
    def read_cache(self):
        pass
    def add_record(self):
        pass


class DepsInstaller:
    def __init__(self):
        self.libs = []
        self.__build_mode = BuildMode.Debug
        self.__wincompiler = WinCompiler.No
        self.__winvsversion = WinVSVersion.No
        self.__depsCache = DepsCache()


    def __process_args(self):
        import argparse
        parser = argparse.ArgumentParser()
        parser.add_argument('-m', "--mode", help='Libraries build mode: Debug = 1, Release = 2',
                            type=int, required=True)
        if get_platform() == PlatformType.Windows:
            parser.add_argument("--wincompiler", type=int, default=2, help='MinGW = 1, Visual Studio = 2')
            parser.add_argument("--winvsversion", type=int, default=2, help='Vs 19 = 1, Vs 22 = 2')
        args = parser.parse_args()
        self.__build_mode = BuildMode(args.mode)
        if get_platform() == PlatformType.Windows:
            if args.wincompiler:
                self.__wincompiler = WinCompiler(args.wincompiler)
            if args.winvsversion:
                self.__winvsversion = WinVSVersion(args.winvsversion)

    def __process_cache(self):
        self.__depsCache.read_cache()


    def __setup(self):
        self.__process_args()
        self.__process_cache()

        currentPlatform = get_platform()

        printColored(bcolors.WARNING, f'Current platform is: {currentPlatform.value}')
        if get_platform() == PlatformType.Windows:
            printColored(bcolors.WARNING, "Freetype library will not installed by this script. "
                                          "Will be using included during CMake.")

        nixLibNames = {
            'glfw': NixLibName('glfw3', 'libglfw3 libglfw3-dev'),
            'spdlog': NixLibName('spdlog', 'libspdlog-dev'),
        }

        libCmakeOptions = {
            'glfw': '-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF',
            'spdlog': '-DSPDLOG_BUILD_EXAMPLE=OFF',
        }

        generator = ''
        if currentPlatform is not PlatformType.Windows:
            generator = 'Unix Makefiles'
        else:
            if self.__wincompiler is WinCompiler.MinGW:
                generator = 'MinGW Makefiles'
            if self.__wincompiler is WinCompiler.VS:
                generator = str(self.__winvsversion)

        for lib in LibType.libs():
            key = lib.value
            cmakeOptions = CMakeOptions(generator,
                                        libCmakeOptions[key],
                                        'mingw32-' if self.__wincompiler is WinCompiler.MinGW else '')
            lib = Lib(key,
                      Consts.getURL(LibType(lib)),
                      nixLibNames[key].MacOSName, nixLibNames[key].LinuxName,
                      cmakeOptions,
                      str(self.__build_mode))
            self.libs.append(lib)

    def __install_core_deps(self):
        deps = functools.reduce(lambda l, r: l + ', ' + r, [lib.name for lib in self.libs])
        print(f"Install dependencies: \n {deps}")
        deps_folder = 'depslibs'
        full_path = os.path.join(os.path.basename(__file__), deps_folder)
        
        if not os.path.exists(os.path.join(os.getcwd(), deps_folder)):
            try:
                os.mkdir(deps_folder)
                os.chdir(deps_folder)
            except OSError:
                printColored(bcolors.FAIL, "Can't create and move to deps folder")
                # TODO(a.raag): exit code statuses
                exit(1)

        for lib in self.libs:
            print('-----------------------------------------------------------')
            printColored(bcolors.HEADER, f"1. Processing {lib.name} library")
            if lib.run():
                printColored(bcolors.HEADER, f"2. Success install {lib.name} library")
                # TODO(a.raag): mark cache lib is installed
            else:
                pass
            print('-----------------------------------------------------------')
        printColored(bcolors.OKGREEN, "All libs installed")

        # print('Uninstall tmp deps folder')
        # try:
        #     os.rmdir(deps_folder)
        #     print('Uninstall - done')
        # except OSError:
        #     printColored(bcolors.FAIL, "Can't uninstall deps folder")
        #     # TODO(a.raag): exit code statuses
        #     exit(1)

    def __install_editor_deps(self):
        # update git submodules
        try:
            cmd = Cmd("git submodule update --init --recursive")
            proc = subprocess.Popen(cmd.getCmd(), shell=True)
            proc.wait()
        except Exception:
            pass
        # install mono experimental

    def run(self):
        self.__setup()
        self.__install_core_deps()
        self.__install_editor_deps()


if __name__ == '__main__':
    installer = DepsInstaller()
    installer.run()
