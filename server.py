import json
import logging
import queue
import socket
import sys
import threading

import captions
from common import BYTEORDER, HEADER_SIZE, HOST, PORT

EXPECTED_CHARACTER = ""


def dispatch_captions(captions_queue: queue.Queue) -> None:
    logger = logging.basicConfig(
        stream=sys.stdout, name="dispatch_captions", level=logging.INFO
    )
    for caption in captions.CAPTIONS:
        logger.debug(f"Putting new caption into queue: {caption}")
        captions_queue.put(caption)
        # time.sleep(random.randrange(0, 10))
    logger.info(f"Completed caption transmission, putting None in queue to terminate.")
    captions_queue.put(None)


def socket_transmission(captions_queue: queue.Queue, conn: socket.socket) -> None:
    logger = logging.basicConfig(
        stream=sys.stdout, name="socket_transmission", level=logging.INFO
    )
    while caption := captions_queue.get():
        logger.debug(f"Received a message from input queue. Message is: {caption}")
        message = json.dumps(caption).encode("utf-8")
        message_len = len(message)
        message_len_bytes = message_len.to_bytes(HEADER_SIZE, BYTEORDER)
        logger.debug(f"Sending message length to socket: {message_len}")
        conn.sendall(message_len_bytes)
        logger.debug(f"Sending message in UTF-8 format to socket")
        conn.sendall(message)
        captions_queue.task_done()
    logger.info(f"Received None from queue, ending thread.")
    captions_queue.task_done()


def main(host: str = HOST, port: int = PORT) -> None:
    captions_queue = queue.Queue()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind((host, port))
        sock.listen()
        conn, addr = sock.accept()
        logging.info(f"Socket connection received from: {addr}")
        with conn:
            ready = input("Press ENTER to begin the experiment.")
            while ready != EXPECTED_CHARACTER:
                ready = input(
                    "Invalid character received. Press ENTER to begin the experiment."
                )
            t1 = threading.Thread(target=dispatch_captions, args=(captions_queue,))
            t2 = threading.Thread(
                target=socket_transmission, args=(captions_queue, conn)
            )
            logging.debug("Starting process_captions_with_serial thread")
            t1.start()
            logging.debug("Starting socket_transmission thread")
            t2.start()

            captions_queue.join()
            logging.info("All tasks put in the queue have been completed.")


if __name__ == "__main__":
    main()
