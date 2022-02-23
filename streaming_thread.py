import json
import socket
import threading
import typing

from captions_thread import Caption, CaptionsThread
from common import BYTEORDER, HEADER_SIZE
from orientation_reading_thread import OrientationReadingThread
from serial_thread import MockSerialThread, SerialThread


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
        captions_thread: CaptionsThread,
        serial_thread: typing.Optional[
            typing.Union[SerialThread, MockSerialThread]
        ] = None,
        orientation_reading_thread: typing.Optional[OrientationReadingThread] = None,
    ):
        threading.Thread.__init__(self)
        if not (serial_thread or orientation_reading_thread):
            raise Exception(
                "Either serial_thread or orientation_reading_thread must be set."
            )
        self.connection = connection
        self.orientation_reading_thread = orientation_reading_thread
        self.serial_thread = serial_thread
        self.captions_thread = captions_thread
        self.last_focused_juror: typing.Optional[
            typing.Literal["juror-a", "juror-b", "juror-c", "jury-foreman"]
        ] = None
        self.last_caption: typing.Optional[Caption] = None

    def focused_juror_from_orientation(self):
        with self.orientation_reading_thread.lock:
            orientation = self.orientation_reading_thread.current_orientation
            # return calculate_focused_juror(orientation)
            print(orientation)

    def focused_juror_from_serial(self):
        with self.serial_thread.lock:
            return self.serial_thread.current_focused_juror

    def run(self) -> None:
        while True:
            focused_juror = (
                # self.focused_juror_from_orientation()
                # if self.orientation_reading_thread
                # else self.focused_juror_from_serial()
                self.focused_juror_from_serial()
            )
            with self.captions_thread.lock:
                caption = self.captions_thread.current_caption
                if (
                    caption == self.last_caption
                    and focused_juror == self.last_focused_juror
                ):
                    continue
                self.last_caption = caption
                self.last_focused_juror = focused_juror
                message = {
                    "message_id": caption.message_id,
                    "chunk_id": caption.chunk_id,
                    "text": caption.text,
                    "speaker_id": caption.speaker_id,
                    "focused_id": focused_juror,
                }
                self.focused_juror_from_orientation()
                msg = json.dumps(message).encode("utf-8")
                msg_len = len(msg)
                msg_len_bytes = msg_len.to_bytes(HEADER_SIZE, BYTEORDER)
                msg_with_header = msg_len_bytes + msg
                self.connection.sendall(msg_with_header)
