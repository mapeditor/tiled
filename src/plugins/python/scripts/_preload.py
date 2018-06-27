import sys
#from tiled.Tiled.LoggingInterface import INFO,ERROR

class _LogCatcher:
    def __init__(self, type):
        self.buffer = ''
        self.type = type

    def flush(self):
        pass

    def write(self, msg):
        self.buffer += msg
        try:
            if self.buffer.endswith('\n'):
                sys._logger.log(self.type, self.buffer)
                self.buffer = ''
        except Exception as ex:
            import traceback
            traceback.print_exc(file=sys._oldstderr)


sys._oldstdout = sys.stdout
sys._oldstderr = sys.stdout
sys.stdout = _LogCatcher(0)
sys.stderr = _LogCatcher(1)

