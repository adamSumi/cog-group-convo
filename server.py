import json
import logging
import multiprocessing
import os
import time
import datetime
import random
import socket
from typing import Callable, Dict, List, Literal, Optional

import rx
from rx import operators as ops
from rx.core.typing import Observable, Observer, Scheduler
from rx.scheduler.threadpoolscheduler import ThreadPoolScheduler
import vlc
import qrcode

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


def load_video_in_vlc(
    video_path: str, shared_vlc_instance: Optional[vlc.Instance] = None
) -> vlc.MediaPlayer:
    vlc_instance: vlc.Instance = (
        vlc.Instance() if not shared_vlc_instance else shared_vlc_instance
    )
    player: vlc.MediaPlayer = vlc_instance.media_player_new()
    media = vlc_instance.media_new(video_path)
    player.set_media(media)
    return player


def get_ip() -> str:
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(("10.255.255.255", 1))
        ip = s.getsockname()[0]
    except Exception:
        ip = "127.0.0.1"
    finally:
        s.close()
    return ip


def render_connection_qrcode(ip: str, port: int) -> None:
    connection_str = ip + ":" + str(port)
    logging.debug(f"Glasses should access address: {connection_str}")
    qr = qrcode.QRCode(
        version=1,
        error_correction=qrcode.ERROR_CORRECT_L,
        box_size=1,
    )
    qr.add_data(connection_str)
    qr.make(fit=True)
    qr.print_ascii()


def main(host: str = HOST, port: int = PORT) -> None:
    """
    Constructs observables from the pre-defined list of captions and the serial monitor input,
    then establishes a TCP socket and waits for connection. Once a socket connection is received,
    begins transmitting messages to the connected socket. The content of the messages depends
    on the reading taken from the serial monitor (see create_message)
    """
    logging.debug("Loading VLC videos")
    players: List[vlc.MediaPlayer] = []
    for i in range(3):  # range(NUM_JURORS):
        players.append(load_video_in_vlc(os.path.join("videos", f"{i+1}.mp4")))
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
        render_connection_qrcode(get_ip(), port)
        conn, addr = sock.accept()
        logging.info(f"Socket connection received from: {addr[0]}:{addr[1]}")
        for player in players:
            player.play()
            player.set_pause(1)
        ready = input("Press ENTER to begin the experiment.")
        while ready != EXPECTED_CHARACTER:
            ready = input(
                "Invalid character received. Press ENTER to begin the experiment."
            )
        for player in players:
            player.set_pause(0)
        time.sleep(1000)
        messages_observable = rx.combine_latest(
            captions_observable,
            serial_monitor_observable,
        ).pipe(ops.map(lambda x: create_message(x[0], x[1])))
        messages_observable.subscribe(
            lambda message: socket_transmission(message, conn)
        )


if __name__ == "__main__":
    main()
