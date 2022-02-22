import queue
import threading
import time
import typing


class Caption(typing.NamedTuple):
    text: str
    message_id: int
    chunk_id: int
    delay: float
    speaker_id: typing.Literal["juror-a", "juror-b", "juror-c", "jury-foreman"]


class CaptionsThread(threading.Thread):
    """
    This thread iterates over the captions provided, one-by-one, waiting for the amount of time indicated by the "delay" value before proceeding to the next caption.
    The `self.current_caption` variable will have the latest caption value.

    REMEMBER TO ACQUIRE/RELEASE THE LOCK BEFORE USE.
    """

    def __init__(self, captions: typing.List[Caption]):
        threading.Thread.__init__(self)
        self.captions = captions
        self.lock = threading.Lock()
        self.current_caption: typing.Optional[Caption] = None

    def run(self) -> None:
        for caption in self.captions:
            time.sleep(caption.delay / 1000)
            with self.lock:
                self.current_caption = caption
