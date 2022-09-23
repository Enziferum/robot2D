import os
import sys
import functools
import enum
import subprocess

GITHUB_URL = 'https://github.com/'
GIT_END = '.git'

"""
    How to add new dep ?
    1. Add Dep to LibType
    2. In Consts update getURL to be able download by git
    3. nixLibName add library. If Empty will be downloaded by git url. Need for packet manager
    4. Add lib cmake options libCmakeOptions if no make empty string
"""

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def printColored(color, text, endColor: bool = True):
    print(f"{color}{text}{bcolors.ENDC if endColor else ''}")


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
    ROBOT2D_EXT = 'robot2D_ext'

    @classmethod
    def libs(cls) -> list:
        return list(cls.__members__.values())


class Consts:
    @staticmethod
    def getURL(lib: LibType) -> str:
        githubUserNames = {
            'glfw': 'glfw',
            'robot2D_ext': 'Enziferum',
            'spdlog': 'gabime'
        }
        return f'{GITHUB_URL}{githubUserNames[lib.value]}/{lib.value}{GIT_END}'


class WinCompiler(enum.IntEnum):
    No = 0
    MinGW = 1
    VS = 2


class WinVSVersion(enum.IntEnum):
    No = 0
    VS_17 = 1
    VS_19 = 2
    VS_22 = 3

    def __str__(self):
        if self.value == self.VS_17:
            return "Visual Studio 15 2017 Win64"
        if self.value == self.VS_19:
            return "Visual Studio 16 2019"
        if self.value == self.VS_22:
            return "Visual Studio 16 2022"


class Cmd:
    def __init__(self, cmd: str):
        self.cmd = cmd

    def getCmd(self) -> str:
        return self.cmd


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
                           f'&& cd build && cmake .. -G "{generator}" {cmake_options} && {os_make}make '
                           f'&& sudo {os_make}make install')
        self.cmds.append(cmake_cmd)

    def __build_library_vs(self, generator, cmake_options=''):
        cmake_cmd = Cmd(f'cd {self.name} && mkdir build '
                           f'&& cd build && cmake .. -G "{generator}" -DCMAKE_CONFIGURATION_TYPES:STRING="{self.configuration}" '
                           f' -DCMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD=ON '
                           f'{cmake_options} && cmake --build .'
                           f' && cmake --install .')
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
                # todo support not only ubuntu
                packman_cmd = 'sudo apt install'
                libname = self.linuxName

            self.cmds.append(Cmd(f'{packman_cmd} {libname}'))
        else:
            git_cmd = f'git clone {self.giturl}'
            self.cmds.append(Cmd(git_cmd))
            if currentPlatform is PlatformType.Windows:
                self.__build_library_vs(self.cmakeOptions.generator,
                                     self.cmakeOptions.options)
                return
            self.__build_library(self.cmakeOptions.generator,
                                 self.cmakeOptions.os_make,
                                 self.cmakeOptions.options)

    def run(self):
        self.__pack_cmds()

        for cmd in self.cmds:
            printColored(bcolors.WARNING, f'Execute command {cmd.getCmd()}')
            proc = subprocess.Popen(cmd.getCmd(), shell=True)
            proc.wait()

    def __str__(self):
        return str(self.__dict__)

class ConfigurationType(enum.IntEnum):
    Debug = 1
    Release = 2
    
    def __str__(self):
        if self.value == self.Debug:
            return "Debug"
        if self.value == self.Release:
            return "Release"

class DepsInstaller:
    def __init__(self):
        self.libs = []
        self.__wincompiler = WinCompiler.No
        self.__winvsversion = WinVSVersion.No

    def __setup(self):
        currentPlatform = get_platform()
        printColored(bcolors.WARNING, f'Current platform is: {currentPlatform.value}')
        # configuration = input("Choose Configuration: \n"
        #                      "1 - Debug \n"
        #                      "2 - Release \n")
        configuration = ConfigurationType(int("2"))
        if get_platform() == PlatformType.Windows:
            compiler = input("Choose Compiler: \n"
                             "1 - MinGW \n"
                             "2 - VS \n")
            self.__wincompiler = WinCompiler(int(compiler))
            if self.__wincompiler == WinCompiler.VS:
                version = input("Choose Version: \n"
                                "1 - Visual Studio 2017 \n"
                                "2 - Visual Studio 2019 \n"
                                "3 - Visual Studio 2022 \n"
                                )
                self.__winvsversion = WinVSVersion(int(version))
        nixLibNames = {
            'glfw': NixLibName('glfw3', 'libglfw3 libglfw3-dev'),
            'spdlog': NixLibName('spdlog', 'libspdlog-dev'),
            'robot2D_ext': NixLibName('', ''),
        }

        libCmakeOptions = {
            'glfw': '-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF',
            'spdlog': '-DSPDLOG_BUILD_EXAMPLE=OFF',
            'robot2D_ext': ''
        }

        generator = ''
        if currentPlatform != PlatformType.Windows:
            generator = 'Unix Makefiles'
        else:
            if self.__wincompiler is WinCompiler.MinGW:
                generator = 'MinGW Makefiles'
            if self.__wincompiler is WinCompiler.VS:
                generator = str(self.__winvsversion)

        for it in LibType.libs():
            key = it.value
            cmakeOptions = CMakeOptions(generator,
                                        libCmakeOptions[key],
                                        'mingw32-' if self.__wincompiler is WinCompiler.MinGW else '')
            lib = Lib(key, Consts.getURL(LibType(it)),
                      nixLibNames[key].MacOSName, nixLibNames[key].LinuxName,
                      cmakeOptions,
                      str(configuration))
            self.libs.append(lib)

    def run(self):
        self.__setup()

        deps = functools.reduce(lambda l, r: l + ', ' + r, [lib.name for lib in self.libs])
        print(f"Install dependencies: \n {deps}")
        deps_folder = 'depslibs'

        if not os.path.exists(os.path.join(os.curdir, deps_folder)):
            os.mkdir(deps_folder)
            os.chdir(deps_folder)

        for lib in self.libs:
            print('-----------------------------------------------------------')
            printColored(bcolors.HEADER, f"1. Processing {lib.name} library")
            lib.run()
            printColored(bcolors.HEADER, f"2. Success install {lib.name} library")
            print('-----------------------------------------------------------')
        printColored(bcolors.OKGREEN, "All libs installed")


if __name__ == '__main__':
    installer = DepsInstaller()
    installer.run()
