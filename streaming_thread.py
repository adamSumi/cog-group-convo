import json
import socket
import threading
import typing

from captions_thread import Caption, CaptionsThread
from common import BYTEORDER, HEADER_SIZE
import flatbuffers
from orientation_reading_thread import OrientationReadingThread
from serial_thread import MockSerialThread, SerialThread
from cog_flatbuffer_definitions.cog import CaptionMessage


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
        builder = flatbuffers.Builder()
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
                print("caption =", caption)
                self.last_focused_juror = focused_juror
                text = builder.CreateString(caption.text)
                speaker_id = builder.CreateString(caption.speaker_id)
                if focused_juror:
                    focused_id = builder.CreateString(focused_juror)
                CaptionMessage.CaptionMessageStart(builder)
                CaptionMessage.AddMessageId(builder, caption.message_id)
                CaptionMessage.AddChunkId(builder, caption.chunk_id)
                CaptionMessage.AddText(builder, text)
                CaptionMessage.AddSpeakerId(builder, speaker_id)
                if focused_juror:
                    CaptionMessage.AddFocusedId(builder, focused_id)
                caption_message = CaptionMessage.End(builder)
                builder.Finish(caption_message)
                buf = builder.Output()

                # self.focused_juror_from_orientation()
                buf = builder.Output()
                self.connection.sendall(len(buf).to_bytes(HEADER_SIZE, BYTEORDER))
                self.connection.sendall(buf)
