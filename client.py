#!/usr/bin/env python3

import socket
from time import sleep, time
from cog_flatbuffer_definitions.cog.CaptionMessage import CaptionMessage
import flatbuffers
from cog_flatbuffer_definitions.cog import OrientationMessage
import random

from common import BYTEORDER, HEADER_SIZE

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    host = input("IP address?")
    port = input("Port?")
    s.connect((host, int(port)))
    while True:
        builder = flatbuffers.Builder()
        OrientationMessage.Start(builder)
        OrientationMessage.AddAzimuth(builder, random.random())
        OrientationMessage.AddPitch(builder, random.random())
        OrientationMessage.AddRoll(builder, random.random())
        orientation_message = OrientationMessage.End(builder)
        builder.Finish(orientation_message)
        buf = builder.Output()
        s.sendall(len(buf).to_bytes(HEADER_SIZE, BYTEORDER))
        s.sendall(buf)
        msg_size = int.from_bytes(s.recv(HEADER_SIZE, socket.MSG_WAITALL), BYTEORDER)
        caption_buf = bytearray(s.recv(msg_size, socket.MSG_WAITALL))
        caption_message = CaptionMessage.GetRootAs(caption_buf)
        print("text =", caption_message.Text(), "speaker_id =", caption_message.SpeakerId(), "focused_id =", caption_message.FocusedId())