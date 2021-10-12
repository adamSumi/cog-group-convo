import json
import logging
import multiprocessing
import time
import datetime
import random
import socket
from typing import Dict, Optional, Tuple, List

import rx
from rx import operators as ops
from rx.core.typing import Observable, Observer, Scheduler
from rx.scheduler.threadpoolscheduler import ThreadPoolScheduler

import captions
from common import BYTEORDER, HEADER_SIZE, HOST, PORT

EXPECTED_CHARACTER = ""

logging.basicConfig(level=logging.DEBUG)

random_juror = lambda: random.choice(
    ["juror-a", "juror-b", "juror-c", "jury-foreman", None]
)


def socket_transmission(
    caption: Dict[str, str], juror: Optional[str], conn: socket.socket
) -> None:
    message = json.dumps(caption).encode("utf-8")
    message_len = len(message)
    message_len_bytes = message_len.to_bytes(HEADER_SIZE, BYTEORDER)
    conn.sendall(message_len_bytes)
    conn.sendall(message)


def serial_monitor(observer: Observer, scheduler: Scheduler) -> str:
    while True:
        observer.on_next(random_juror())
        time.sleep(1)


def build_delayed_caption_obs(caption: Dict[str, str]) -> Observable:
    return rx.just(caption).pipe(
        ops.delay(datetime.timedelta(milliseconds=random.randint(0, 1000)))
    )


def main(host: str = HOST, port: int = PORT) -> None:
    # captions_queue = queue.Queue()
    scheduler = ThreadPoolScheduler(multiprocessing.cpu_count())
    captions_observable: Observable = rx.concat_with_iterable(
        build_delayed_caption_obs(caption) for caption in captions.CAPTIONS
    )
    serial_monitor_observable = rx.create(serial_monitor).pipe(
        ops.subscribe_on(scheduler),
        ops.distinct_until_changed()
    )
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((host, port))
        sock.listen()
        conn, addr = sock.accept()
        logging.info(f"Socket connection received from: {addr}")
        ready = input("Press ENTER to begin the experiment.")
        while ready != EXPECTED_CHARACTER:
            ready = input(
                "Invalid character received. Press ENTER to begin the experiment."
            )
        captions_and_serial_observable = rx.combine_latest(
            captions_observable,
            serial_monitor_observable,
        )
        captions_and_serial_observable.subscribe(
            lambda x: socket_transmission(x[0], x[1], conn)
        )


if __name__ == "__main__":
    main()
