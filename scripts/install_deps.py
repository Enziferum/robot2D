import os
import sys
import functools
import enum


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
        if self.value == self.VS_17:
            return "Visual Studio 16 2019"
        if self.value == self.VS_17:
            return "Visual Studio 16 2022"



def printColored(color, text, endColor: bool = True):
    print(f"{color}{text}{bcolors.ENDC if endColor else ''}")


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

    return platforms[sys.platform]


class Cmd:
    def getCmd(self) -> str:
        raise NotImplementedError


class NixCmd(Cmd):
    def __init__(self, cmd: str):
        self.cmd = cmd

    def getCmd(self) -> str:
        return self.cmd


class Lib:
    def __init__(self, name: str):
        self.name = name
        self.options = ''
        self.cmds = []

    def run(self):
        for cmd in self.cmds:
            printColored(bcolors.WARNING, f'Execute command {cmd.getCmd()}')
            import subprocess
            proc = subprocess.Popen(cmd.getCmd(), shell=True, executable="/bin/zsh")
            proc.wait()

    def __str__(self):
        return str(self.__dict__)


class DepsInstaller:
    def __init__(self):
        self.libs = []
        self.__setup()
        self.__wincompiler = WinCompiler.No
        self.__winvsversion = WinVSVersion.No

    def __build_library(self, lib, giturl, libname, generator, os_make='', cmake_options=''):
        git_cmd = f'git clone {giturl}'
        lib.cmds.append(NixCmd(git_cmd))
        cmake_cmd = NixCmd(f'cd {libname} && mkdir build '
                           f'&& cd build && cmake .. -G "{generator}" {cmake_options} && {os_make}make '
                           f'&& {os_make}make install')
        lib.cmds.append(cmake_cmd)

    def __build_library_vs(self, lib, giturl, libname, generator, cmake_options=''):
        git_cmd = f'git clone {giturl}'
        lib.cmds.append(NixCmd(git_cmd))
        cmake_cmd = NixCmd(f'cd {libname} && mkdir build '
                           f'&& cd build && cmake .. -G "{generator}" -DCMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD=ON '
                           f'{cmake_options} && cmake --build .'
                           f'&& cmake install .')
        lib.cmds.append(cmake_cmd)

    def __setupGLFW(self):
        currentPlatform = get_platform()
        libname = 'glfw'
        lib = Lib(libname)

        if currentPlatform == 'Mac':
            cmd = NixCmd("brew install glfw3")
            lib.cmds.append(cmd)

        if currentPlatform == 'Linux':
            cmd = NixCmd("apt install libglfw3 libglfw3-dev")
            lib.cmds.append(cmd)

        if currentPlatform == 'Windows':
            giturl = 'https://github.com/glfw/glfw.git'
            if self.__wincompiler == WinCompiler.MinGW:
                self.__build_library(lib, giturl, libname, 'MinGW Makefiles', 'mingw32-',
                                     '-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF')
                return
            if self.__wincompiler == WinCompiler.VS:
                self.__build_library_vs(lib, giturl, libname, str(self.__winvsversion),
                                     '-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF')
                return

        self.libs.append(lib)

    def __setupRobot2D_ext(self):
        currentPlatform = get_platform()
        libname = 'robot2D_ext'
        lib = Lib(libname)

        giturl = 'https://github.com/Enziferum/robot2D_ext.git'
        if currentPlatform == 'Mac' or currentPlatform == 'Linux':
            generator = 'Unix Makefiles'
            self.__build_library(lib, giturl, libname, generator)
            return
        if currentPlatform == 'Windows':
            if self.__wincompiler == WinCompiler.MinGW:
                self.__build_library(lib, giturl, libname, 'MinGW Makefiles', 'mingw32-')
                return
            if self.__wincompiler == WinCompiler.VS:
                self.__build_library_vs(lib, giturl, libname, str(self.__winvsversion))
                return

        self.libs.append(lib)

    def __setupSpdlog(self):
        currentPlatform = get_platform()
        libname = 'spdlog'
        lib = Lib(libname)

        if currentPlatform == 'Mac':
            cmd = NixCmd('brew install spdlog')
            lib.cmds.append(cmd)

        if currentPlatform == 'Linux':
            cmd = NixCmd('apt install libspdlog-dev')
            lib.cmds.append(cmd)

        if currentPlatform == 'Windows':
            giturl = 'https://github.com/gabime/spdlog.git'
            if self.__wincompiler == WinCompiler.MinGW:
                self.__build_library(lib, giturl, libname, 'MinGW Makefiles', 'mingw32-')
                return
            if self.__wincompiler == WinCompiler.VS:
                self.__build_library_vs(lib, giturl, libname, str(self.__winvsversion))
                return

        self.libs.append(lib)

    def __setupFreetype(self):
        libname = 'freetype'
        currentPlatform = get_platform()
        lib = Lib(libname)

        if currentPlatform == 'Mac':
            cmd = NixCmd('brew install freetype')
            lib.cmds.append(cmd)

        if currentPlatform == 'Linux':
            cmd = NixCmd('apt install libfreetype6 libfreetype6-dev')
            lib.cmds.append(cmd)

        if currentPlatform == 'Windows':
            printColored(bcolors.FAIL, "Can't install freetype directly in Windows")

        self.libs.append(lib)

    def __setup(self):
        self.__setupGLFW()
        self.__setupSpdlog()
        self.__setupRobot2D_ext()
        self.__setupFreetype()

    def run(self):
        printColored(bcolors.WARNING, f'Current platform is: {get_platform()}')
        if get_platform() == 'Windows':
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
