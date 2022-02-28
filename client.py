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
        # builder = flatbuffers.Builder()
        # OrientationMessage.Start(builder)
        # OrientationMessage.AddAzimuth(builder, -1.0)
        # OrientationMessage.AddPitch(builder, 2.0)
        # OrientationMessage.AddRoll(builder, -3.0)
        # orientation_message = OrientationMessage.End(builder)
        # builder.Finish(orientation_message)
        # buf = builder.Output()
        # print("Length of orientation message:", len(buf))
        # s.sendall(buf)
        caption_buf = s.recv(1024)
        print(len(caption_buf))
        # print(caption_buf)
        caption_message = CaptionMessage.GetRootAs(caption_buf, 0)
        print(
            "text =",
            caption_message.Text(),
            "speaker_id =",
            caption_message.SpeakerId(),
            "focused_id =",
            caption_message.FocusedId(),
        )
