#!/usr/bin/env python3

import socket
import json
from pprint import pprint
from common import BYTEORDER, HEADER_SIZE, HOST, PORT

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    while msg_len_bytes := s.recv(HEADER_SIZE):
        data = s.recv(int.from_bytes(msg_len_bytes, BYTEORDER))
        data_as_str = data.decode("utf-8")
        pprint(json.loads(data_as_str))
        print("-----------------------------")
