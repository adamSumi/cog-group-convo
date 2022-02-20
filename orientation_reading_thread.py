import socket
import struct
import threading

from common import HEADER_SIZE


class OrientationReadingThread(threading.Thread):
    def __init__(self, connection: socket.socket):
        threading.Thread.__init__(self)
        self.connection = connection

    def run(self):
        while True:
            # MSG_WAITALL only works on Unix-based systems (see https://linux.die.net/man/2/recv).
            # Probably should use a cross-platform solution for Windows devs, but this does what
            # we need (wait until buffer is full before returning) with minimal code writing.
            read_value = self.connection.recv(HEADER_SIZE * 3, socket.MSG_WAITALL)
            values = struct.unpack("fff", read_value)
            print(values)
