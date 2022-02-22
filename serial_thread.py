import random
import threading
import time
import typing
import serial


class SerialThread(threading.Thread):
    """
    This thread reads from the serial monitor, and will update the currently focused juror based on the serial monitor's output. The currently-focused juror can be found at currently_focused_juror

    REMEMBER TO ACQUIRE/RELEASE THE LOCK BEFORE USE.
    """

    def __init__(self, serial: serial.Serial):
        threading.Thread.__init__(self)
        self.serial = serial
        self.lock = threading.Lock()
        self.current_focused_juror: typing.Optional[
            typing.Literal["juror-a", "juror-b", "juror-c", "jury-foreman"]
        ] = None

    def run(self) -> None:
        with self.serial as ser:
            while True:
                juror_val = ser.readline().decode("ascii").strip()
                focused_juror = None
                if juror_val == "0":
                    focused_juror = "juror-a"
                if juror_val == "1":
                    focused_juror = "juror-b"
                if juror_val == "2":
                    focused_juror = "juror-c"
                if juror_val == "3":
                    focused_juror = "jury-foreman"
                if juror_val == "9":
                    pass
                with self.lock:
                    self.current_focused_juror = focused_juror


class MockSerialThread(threading.Thread):
    """
    This thread can be used to simulate serial monitor behavior when a developer does not have access to the hardware.
    
    REMEMBER TO ACQUIRE/RELEASE THE LOCK BEFORE USE.
    """
    def __init__(self):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.current_focused_juror: typing.Optional[
            typing.Literal["juror-a", "juror-b", "juror-c", "jury-foreman"]
        ] = None

    def run(self) -> None:
        while True:
            time.sleep((random.random() * 4) + 0.5)
            with self.lock:
                self.current_focused_juror = random.choice(
                    ["juror-a", "juror-b", "juror-c", "jury-foreman", None]
                )
