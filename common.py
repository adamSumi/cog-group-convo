from typing import Literal

HEADER_SIZE = 4
BYTEORDER = "big"

HOST = "127.0.0.1"
PORT = 65432
JurorId = Literal["juror-a", "juror-b", "juror-c", "jury-foreman", None]
