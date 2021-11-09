from enum import IntEnum
from typing import Literal

HEADER_SIZE = 4
BYTEORDER = "big"

HOST = "127.0.0.1"
PORT = 65432
JurorId = Literal["juror-a", "juror-b", "juror-c", "jury-foreman", None]

class RenderingMethod(IntEnum):
    MONITOR_ONLY = 1
    GLOBAL_ONLY = 2
    MONITOR_AND_GLOBAL = 3
    GLOBAL_WITH_DIRECTION_INDICATORS = 4
    WHO_SAID_WHAT = 5
    MONITOR_AND_GLOBAL_WITH_DIRECTION_INDICATORS = 6
    FOCUSED_SPEAKER_ONLY = 8
    FOCUSED_SPEAKER_AND_GLOBAL = 9
