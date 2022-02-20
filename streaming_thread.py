import socket
import threading
import time
import typing

from orientation_reading_thread import OrientationReadingThread
from captions_thread import Caption, CaptionsThread


def calculate_focused_juror(
    orientation: typing.Tuple[float, float, float]
) -> typing.Optional[typing.Literal["juror-a", "juror-b", "juror-c", "jury-foreman"]]:
    return "juror-a"


class StreamingThread(threading.Thread):
    """
    This thread reads from the provided OrientationReadingThread and CaptionsThread, combines their data, and transmits that data to the socket connection.
    """

    def __init__(
        self,
        connection: socket.socket,
        orientation_reading_thread: OrientationReadingThread,
        captions_thread: CaptionsThread,
    ):
        threading.Thread.__init__(self)
        self.connection = connection
        self.orientation_reading_thread = orientation_reading_thread
        self.captions_thread = captions_thread
        self.last_focused_juror: typing.Optional[
            typing.Literal["juror-a", "juror-b", "juror-c", "jury-foreman"]
        ] = None
        self.last_caption: typing.Optional[Caption] = None

    def run(self) -> None:
        while True:
            with self.orientation_reading_thread.lock:
                orientation = self.orientation_reading_thread.current_orientation
                focused_juror = calculate_focused_juror(orientation)
                with self.captions_thread.lock:
                    caption = self.captions_thread.current_caption
                    if (
                        caption == self.last_caption
                        and focused_juror == self.last_focused_juror
                    ):
                        continue
                    self.last_caption = caption
                    self.last_focused_juror = focused_juror
                    print(caption, focused_juror)
