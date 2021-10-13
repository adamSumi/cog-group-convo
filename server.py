import json
import logging
import multiprocessing
import time
import datetime
import random
import socket
from typing import Callable, Dict, Literal, Optional

import rx
from rx import operators as ops
from rx.core.typing import Observable, Observer, Scheduler
from rx.scheduler.threadpoolscheduler import ThreadPoolScheduler
import vlc

import captions
from common import BYTEORDER, HEADER_SIZE, HOST, PORT

EXPECTED_CHARACTER = ""
NUM_JURORS = 4

logging.basicConfig(level=logging.DEBUG)

random_juror: Callable[
    [], Literal["juror-a", "juror-b", "juror-c", "jury-foreman", None]
] = lambda: random.choice(["juror-a", "juror-b", "juror-c", "jury-foreman", None])


def create_message(
    caption: Dict[str, str], juror_being_looked_at: Optional[str]
) -> Dict[str, str]:
    logging.debug(f"juror={juror_being_looked_at}, caption['id']={caption['id']}")
    message = {
        "text": caption["text"] if juror_being_looked_at == caption["id"] else "CLEAR",
        "id": caption["id"],
    }
    logging.debug(f"message={message}")
    return message


def socket_transmission(message: Dict[str, str], connection: socket.socket) -> None:
    """
    Serializes the given message and transmits two messages to the given connection:
    1. The size of the message (a "header" of sorts)
    2. The serialized message.
    """
    msg = json.dumps(message).encode("utf-8")
    msg_len = len(msg)
    msg_len_bytes = msg_len.to_bytes(HEADER_SIZE, BYTEORDER)
    logging.debug(f"Sending msg length: {msg_len_bytes}")
    connection.sendall(msg_len_bytes)
    logging.debug(f"Sending msg")
    connection.sendall(msg)
    logging.debug(f"Socket transmission completed.")


def serial_monitor(
    observer: Observer, scheduler: Scheduler
) -> Literal["juror-a", "juror-b", "juror-c", "jury-foreman", None]:
    """
    Reads from the serial monitor, and emits the read value to the provided observer.
    """
    while True:
        observer.on_next(random_juror())
        time.sleep(1)


def build_delayed_caption_obs(caption: Dict[str, str]) -> Observable:
    """
    Takes a caption, and constructs an observable sequence comprised of one caption,
    delayed by a given amount of time.
    TODO: Replace random, hard-coded delay with a delay included in the caption.
    """
    return rx.just(caption).pipe(
        ops.delay(datetime.timedelta(milliseconds=random.randint(0, 1000)))
    )


def main(host: str = HOST, port: int = PORT) -> None:
    """
    Constructs observables from the pre-defined list of captions and the serial monitor input,
    then establishes a TCP socket and waits for connection. Once a socket connection is received,
    begins transmitting messages to the connected socket. The content of the messages depends
    on the reading taken from the serial monitor (see create_message)
    """
    logging.debug("Loading VLC videos")
    vlc_instance = vlc.Instance()
    players = [vlc_instance.media_player_new() for _ in range(NUM_JURORS)]
    juror_videos = [
        vlc_instance.media_new(f"videos/{i}.mp4") for i in range(NUM_JURORS)
    ]
    for i, video in enumerate(juror_videos):
        players[i].set_media(video)
    scheduler = ThreadPoolScheduler(multiprocessing.cpu_count())
    captions_observable: Observable = rx.concat_with_iterable(
        build_delayed_caption_obs(caption) for caption in captions.CAPTIONS
    )
    serial_monitor_observable = rx.create(serial_monitor).pipe(
        ops.subscribe_on(scheduler), ops.distinct_until_changed()
    )
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((host, port))
        sock.listen()
        conn, addr = sock.accept()
        logging.info(f"Socket connection received from: {addr[0]}:{addr[1]}")
        ready = input("Press ENTER to begin the experiment.")
        while ready != EXPECTED_CHARACTER:
            ready = input(
                "Invalid character received. Press ENTER to begin the experiment."
            )
        messages_observable = rx.combine_latest(
            captions_observable,
            serial_monitor_observable,
        ).pipe(ops.map(create_message))
        messages_observable.subscribe(
            lambda message: socket_transmission(message, conn)
        )
        conn.close()


if __name__ == "__main__":
    main()
