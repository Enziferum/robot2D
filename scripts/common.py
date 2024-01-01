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


class Cmd:
    def __init__(self, cmd: str):
        self.cmd = cmd

    def getCmd(self) -> str:
        return self.cmd