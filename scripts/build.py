import urllib
import subprocess
import common
import install_deps

mono_url_path = "https://download.mono-project.com/archive/6.12.0/windows-installer/mono-6.12.0.199-x64-0.msi"



class Cmd:
    def __init__(self, cmd: str):
        self.cmd = cmd

    def getCmd(self) -> str:
        return self.cmd

def update_submodules():
    proc = subprocess.Popen(cmd.getCmd(), shell=True)
    proc.wait()

def download_mono():
    proc = subprocess.Popen(cmd.getCmd(), shell=True)
    proc.wait()

def build_editor():
    proc = subprocess.Popen(cmd.getCmd(), shell=True)
    proc.wait()


if __name__ == '__main__':

    update_submodules()
    download_mono()
    build_editor()
