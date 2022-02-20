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
    def __init__(self, captions: typing.List[Caption]):
        threading.Thread.__init__(self)
        self.captions = captions
        self.lock = threading.Lock()
        self.current_caption: typing.Optional[Caption] = None

    def run(self) -> None:
        for caption in self.captions:
            time.sleep(caption.delay / 1000)
            self.lock.acquire()
            self.current_caption = caption
            self.lock.release()
