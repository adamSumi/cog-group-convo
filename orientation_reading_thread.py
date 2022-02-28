import json
import socket
import threading
from typing import Optional, Tuple
import flatbuffers
from cog_flatbuffer_definitions.cog import OrientationMessage

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
        self.current_orientation: Optional[OrientationMessage.OrientationMessage] = None

    def run(self):
        while True:
            # MSG_WAITALL only works on Unix-based systems (see https://linux.die.net/man/2/recv).
            # Probably should use a cross-platform solution for Windows devs, but this does what
            # we need (wait until buffer is full before returning) with minimal code writing.
            orientation_msg = bytearray(self.connection.recv(1024, socket.MSG_WAITALL))
            with self.lock:
                self.current_orientation = (
                    OrientationMessage.OrientationMessage.GetRootAs(orientation_msg, 0)
                )
