import json
import socket
import struct
import threading
from typing import Optional, Tuple

from common import BYTEORDER, HEADER_SIZE


class OrientationReadingThread(threading.Thread):
    """
    This thread reads three float values from the connected socket, which should represent the azimuth, pitch, and yaw of the connected device.
    The current orientation value can be found from self.current_orientation

    REMEMBER TO ACQUIRE/RELEASE THE LOCK DURING USE.
    """

    def __init__(self, connection: socket.socket):
        threading.Thread.__init__(self)
        self.connection = connection
        self.lock = threading.Lock()
        self.current_orientation: Optional[Tuple[float, float, float]] = None

    def run(self):
        while True:
            # MSG_WAITALL only works on Unix-based systems (see https://linux.die.net/man/2/recv).
            # Probably should use a cross-platform solution for Windows devs, but this does what
            # we need (wait until buffer is full before returning) with minimal code writing.
            msg_size = int.from_bytes(self.connection.recv(HEADER_SIZE, socket.MSG_WAITALL), BYTEORDER)
            orientation_msg = self.connection.recv(msg_size, socket.MSG_WAITALL)
            orientation_message = json.loads(orientation_msg.decode('utf-8'))
            with self.lock:
                azimuth = orientation_message["azimuth"]
                pitch = orientation_message["pitch"]
                roll = orientation_message["roll"]
                self.current_orientation = (azimuth, pitch, roll)
