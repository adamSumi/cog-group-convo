import argparse
import datetime
import json
import logging
import multiprocessing
import os
import random
import socket
import time
from typing import Any, Callable, Dict, Literal
import traceback

import psutil
import qrcode
import rx
import serial
import serial.tools.list_ports
from rx import operators as ops
from rx.core.typing import Observable, Observer, Scheduler
from rx.scheduler.threadpoolscheduler import ThreadPoolScheduler
import vlc

from common import BYTEORDER, HEADER_SIZE, PORT, JurorId, RenderingMethod
from videos import get_audio_devices, play_video

EXPECTED_CHARACTER = ""

JUROR_A_VIDEOS = [os.path.join("videos", f"juror-a.{i}.mp4") for i in range(1, 5)]
JUROR_B_VIDEOS = [os.path.join("videos", f"juror-b.{i}.mp4") for i in range(1, 5)]
JUROR_C_VIDEOS = [os.path.join("videos", f"juror-c.{i}.mp4") for i in range(1, 5)]
JURY_FOREMAN_VIDEOS = [
    os.path.join("videos", f"jury-foreman.{i}.mp4") for i in range(1, 5)
]
# JUROR_A_VIDEO = os.path.join("videos", "juror-a.mp4")
# JUROR_B_VIDEO = os.path.join("videos", "juror-b.mp4")
# JUROR_C_VIDEO = os.path.join("videos", "juror-c.mp4")
# JURY_FOREMAN_VIDEO = os.path.join("videos", "jury-foreman.mp4")

JUROR_A_CAPTIONS = [
    f'file://{os.path.abspath(os.path.join("captions", f"juror-a.{i}.vtt"))}'
    for i in range(1, 5)
]
JUROR_B_CAPTIONS = [
    f'file://{os.path.abspath(os.path.join("captions", f"juror-b.{i}.vtt"))}'
    for i in range(1, 5)
]
JUROR_C_CAPTIONS = [
    f'file://{os.path.abspath(os.path.join("captions", f"juror-c.{i}.vtt"))}'
    for i in range(1, 5)
]
JURY_FOREMAN_CAPTIONS = [
    f'file://{os.path.abspath(os.path.join("captions", f"jury-foreman.{i}.vtt"))}'
    for i in range(1, 5)
]

caption_visibility = multiprocessing.Event()

# JUROR_A_AUDIO_OUTPUT = b"alsa_output.pci-0000_00_1f.3.analog-stereo"
# JUROR_B_AUDIO_OUTPUT = b"bluez_sink.28_11_A5_D9_D9_30.a2dp_sink"
# JUROR_C_AUDIO_OUTPUT = b"alsa_output.pci-0000_00_1f.3.analog-stereo"
# JURY_FOREMAN_AUDIO_OUTPUT = b"bluez_sink.28_11_A5_D9_D9_30.a2dp_sink"

CONFIGURATION = [
    (
        caption_visibility,
        # JUROR_A_AUDIO_OUTPUT,
        False,
    ),
    (
        caption_visibility,
        # JUROR_B_AUDIO_OUTPUT,
        False,
    ),
    (
        caption_visibility,
        # JUROR_C_AUDIO_OUTPUT,
        False,
    ),
    (
        caption_visibility,
        # JURY_FOREMAN_AUDIO_OUTPUT,
        False,
    ),
]

logging.basicConfig(level=logging.DEBUG)


def create_message(caption: Dict[str, Any], juror_being_looked_at) -> Dict[str, Any]:
    logging.debug(
        f"juror={juror_being_looked_at}, caption['speaker_id']={caption['speaker_id']}"
    )
    message = {
        "message_id": caption["message_id"],
        "chunk_id": caption["chunk_id"],
        "text": caption["text"],
        "speaker_id": caption["speaker_id"],
        "focused_id": juror_being_looked_at,
    }
    # logging.debug(f"message={message}")
    return message


def socket_transmission(message: Dict[str, Any], connection: socket.socket) -> None:
    """
    Serializes the given message and transmits one message to the given connection with
    The size of the message (a "header" of sorts) and The serialized message.
    """
    msg = json.dumps(message).encode("utf-8")
    msg_len = len(msg)
    msg_len_bytes = msg_len.to_bytes(HEADER_SIZE, BYTEORDER)
    msg_with_header = msg_len_bytes + msg
    connection.sendall(msg_with_header)


def mock_serial_monitor(observer: Observer, scheduler: Scheduler) -> None:
    while True:
        time.sleep((random.random() * 4) + 0.5)
        observer.on_next(
            random.choice(["juror-a", "juror-b", "juror-c", "jury-foreman", None])
            # "juror-a"
        )


def serial_monitor(
    selected_serial: serial.Serial,
) -> Callable[[Observer, Scheduler], None]:
    """
    Reads from the serial monitor, and emits the read value to the provided observer.
    """

    def _serial_monitor(observer: Observer, scheduler: Scheduler):
        with selected_serial as ser:
            while True:
                juror_val = ser.readline().decode("ascii").strip()
                if juror_val == "0":
                    observer.on_next("juror-a")
                if juror_val == "1":
                    observer.on_next("juror-b")
                if juror_val == "2":
                    observer.on_next("juror-c")
                if juror_val == "3":
                    observer.on_next("jury-foreman")
                if juror_val == "9":
                    observer.on_next(None)

    return _serial_monitor


def update_caption_visibility(
    juror_captions_visible: multiprocessing.Event,
    juror_id: JurorId,
):
    def action(message: Dict[str, Any]):
        if (
            message["speaker_id"] == message["focused_id"]
            and message["speaker_id"] == juror_id
        ):
            juror_captions_visible.set()
        else:
            juror_captions_visible.clear()
        return message

    return action


def build_delayed_caption_obs(caption: Dict[str, Any]) -> Observable:
    """
    Takes a caption, and constructs an observable sequence comprised of one caption,
    delayed by a given amount of time.
    """
    return rx.just(caption).pipe(
        ops.delay(datetime.timedelta(milliseconds=caption["delay"]))
    )


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


def render_connection_qrcode(
    ip: str, port: int, rendering_method: RenderingMethod
) -> None:
    connection_str = ip + ":" + str(port)
    logging.debug(f"Glasses should access address: {connection_str}")
    qr = qrcode.QRCode(
        version=1,
        error_correction=qrcode.ERROR_CORRECT_L,
        box_size=10,
    )
    qr.add_data(f"{connection_str} {rendering_method.value}")
    img = qr.make(fit=True)
    # qr.print_ascii()
    img = qr.make_image(fill_color="black", back_color="white")
    logging.debug("Showing image...")
    img.show()
    logging.debug("Image shown.")
    # return img


def select_serial_port() -> serial.Serial:
    serial_ports = serial.tools.list_ports.comports(include_links=True)
    print(f"Found {len(serial_ports)} serial ports. Which would you like to choose?")
    for i, port in enumerate(serial_ports):
        print(f"[{i}]: {port.device} ({port.description})")
    selection = input()
    while (
        not selection.isnumeric()
        or int(selection) < 0
        or int(selection) >= len(serial_ports)
    ):
        selection = input(f"Please enter a value from 0-{len(serial_ports)}.")
    selected_port = serial_ports[int(selection)]
    return serial.Serial(port=selected_port.device)


def close_qrcode():
    # After connection is established, kill program displaying qrcode
    for proc in psutil.process_iter():
        # change name on ubuntu
        if proc.name() == "Microsoft.Photos.exe" or proc.name() in [
            "display",
            "eog",
            "xv",
        ]:
            proc.kill()


def main(
    host: str,
    port: int,
    rendering_method: RenderingMethod,
    partition: Literal[1, 2, 3, 4],
    for_testing: bool,
) -> None:
    """
    Constructs observables from the pre-defined list of captions and the serial monitor input,
    then establishes a TCP socket and waits for connection. Once a socket connection is received,
    begins transmitting messages to the connected socket. The content of the messages depends
    on the reading taken from the serial monitor (see create_message)
    """
    scheduler = ThreadPoolScheduler(multiprocessing.cpu_count())
    # return

    if for_testing:
        configured_serial_monitor = mock_serial_monitor
    else:
        selected_serial_port = select_serial_port()
        configured_serial_monitor = serial_monitor(selected_serial_port)

    merged_captions = json.load(
        open(os.path.join("captions", f"merged_captions.{partition}.json"), "r")
    )
    merged_captions_observable = rx.merge(
        *(build_delayed_caption_obs(caption) for caption in merged_captions)
    )
    serial_monitor_observable: Observable[JurorId] = rx.create(
        configured_serial_monitor
    ).pipe(ops.subscribe_on(scheduler), ops.distinct_until_changed())
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((host, port))
        sock.listen()
        render_connection_qrcode(get_ip(), port, rendering_method=rendering_method)

        conn, addr = sock.accept()
        logging.info(f"Socket connection received from: {addr[0]}:{addr[1]}")

        close_qrcode()

        messages_observable = rx.combine_latest(
            merged_captions_observable,
            serial_monitor_observable,
        ).pipe(ops.map(lambda x: create_message(x[0], x[1])))

        ready_to_start_playback = multiprocessing.Event()

        video_processes = []
        video_paths = (
            JUROR_A_VIDEOS[partition - 1],
            JUROR_B_VIDEOS[partition - 1],
            JUROR_C_VIDEOS[partition - 1],
            JURY_FOREMAN_VIDEOS[partition - 1],
        )
        captions_paths = (
            JUROR_A_CAPTIONS[partition - 1],
            JUROR_B_CAPTIONS[partition - 1],
            JUROR_C_CAPTIONS[partition - 1],
            JURY_FOREMAN_CAPTIONS[partition - 1],
        )
        for (
            video_path,
            captions_path,
            (
                caption_visibility,
                # audio_output,
                is_muted,
            ),
        ) in zip(video_paths, captions_paths, CONFIGURATION):
            video_processes.append(
                multiprocessing.Process(
                    target=play_video,
                    args=(
                        video_path,
                        ready_to_start_playback,
                        captions_path,
                        caption_visibility,
                        # audio_output,
                        is_muted,
                    ),
                )
            )
        for video_process in video_processes:
            video_process.start()
        ready = input("Press ENTER to begin the experiment.")
        while ready != EXPECTED_CHARACTER:
            ready = input(
                "Invalid character received. Press ENTER to begin the experiment."
            )
        if rendering_method in (
            RenderingMethod.MONITOR_ONLY,
            RenderingMethod.MONITOR_AND_GLOBAL,
            RenderingMethod.MONITOR_AND_GLOBAL_WITH_DIRECTION_INDICATORS,
        ):
            caption_visibility.set()

        messages_observable.subscribe(
            lambda message: socket_transmission(message, conn),
            on_error=lambda e: traceback.print_tb(e.__traceback__),
        )
        ready_to_start_playback.set()

        for video_process in video_processes:
            video_process.join()


if __name__ == "__main__":
    os.environ["VLC_VERBOSE"] = str("-1")
    parser = argparse.ArgumentParser(
        description="Server for transmitting captions to a device on the same network."
    )
    parser.add_argument(
        "rendering_method",
        help="The rendering method to use to render captions on the other device.",
        type=lambda x: RenderingMethod(int(x)),
    )
    parser.add_argument(
        "partition",
        help="Which part of the video to use (1 = first 2.5 minutes, 2 = second 2.5 minutes, etc.",
        choices=range(1, 5),
        type=int,
    )
    parser.add_argument(
        "--host", type=str, default=get_ip(), help="The host IP to bind to."
    )
    parser.add_argument(
        "--port", type=int, default=PORT, help="The port to run this server on."
    )
    parser.add_argument(
        "--for_testing",
        action="store_true",
        help="Whether this server should be trying to connect over serial ports or not.",
    )
    args = parser.parse_args()
    main(args.host, args.port, args.rendering_method, args.partition, args.for_testing)