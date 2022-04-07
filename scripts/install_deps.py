import os
import sys
import functools


def get_platform():
    platforms = {
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


class NixCmd:
    def __init__(self):
        pass


class FolderCmd:
    def __init__(self):
        self.folder_name = 'build'

    def a(self, enter_name: str):
        cmd = f'cd{enter_name} && mkdir {self.folder_name} && cd {self.folder_name}'


class CmakeCmd:
    def __init__(self):
        pass


class Lib:
    def __init__(self, name: str):
        self.name = name
        self.options = ''
        self.cmds = []

    def run(self):
        for cmd in self.cmds:
            print(os.popen(cmd.getCmd()).read())

    def __str__(self):
        return str(self.__dict__)


class DepsInstaller:
    def __init__(self):
        self.libs = []
        self.__setup()

    def __setupGLFW(self):
        currentPlatform = get_platform()

        lib = Lib('glfw')
        giturl = 'https://github.com/glfw/glfw.git'
        if currentPlatform == 'Mac':
            pass

        if currentPlatform == 'Linux':
            pass

        if currentPlatform == 'Windows':
            pass

        #self.libs.append(lib)

    def __setupRobot2D_ext(self):
        currentPlatform = get_platform()
        lib = Lib('robot2D_ext')

        giturl = 'https://github.com/Enziferum/robot2D_ext.git'
        git_cmd = f'git clone {giturl}'

        generator = ''
        os_make = 'mingw32-'

        if currentPlatform == 'Mac' or currentPlatform == 'Linux':
            generator = 'Unix Makefiles'
        if currentPlatform == 'Windows':
            generator = 'MinGW Makefiles'

        cmake_cmd = f'cmake .. -G "{generator}" && {os_make}make && {os_make}make install'

        self.libs.append(lib)

    def __setupSpdlog(self):
        currentPlatform = get_platform()
        lib = Lib('spdlog')
        giturl = 'https://github.com/gabime/spdlog.git'
        if currentPlatform == 'Mac':
            cmd = 'brew install spdlog'

        if currentPlatform == 'Linux':
            cmd = 'apt install libspdlog'

        if currentPlatform == 'Windows':
            git_cmd = f'git clone '
            cmake_cmd = 'cmake .. -G '
        #self.libs.append(lib)

    def __setupFreetype(self):
        currentPlatform = get_platform()
        lib = Lib('Freetype')

        if currentPlatform == 'Mac':
            cmd = 'brew install freetype'

        if currentPlatform == 'Linux':
            cmd = 'apt install libfreetype libfreetype-dev'

        if currentPlatform == 'Windows':
            pass

        #self.libs.append(lib)

    def __setup(self):
        self.__setupGLFW()
        self.__setupRobot2D_ext()
        self.__setupSpdlog()
        self.__setupFreetype()

    def run(self):
        print(f'Current platform is {get_platform()}')

        deps = functools.reduce(lambda l, r: l + ', ' + r, [lib.name for lib in self.libs])
        print("Install dependencies:\n", deps)

        for lib in self.libs:
            print('-----------------------------------------------------------')
            print(f"1. Processing {lib.name} library")
            lib.run()
            print(f"2. Success install {lib.name} library")
            print('-----------------------------------------------------------')
        print("All libs installed")


if __name__ == '__main__':
    installer = DepsInstaller()
    installer.run()
