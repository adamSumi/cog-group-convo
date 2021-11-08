import os
import webvtt
from collections import deque
from typing import Iterable, List, Dict, Tuple
from pprint import pprint
import json

from webvtt.structures import Caption

juror_a = webvtt.read(os.path.join("captions", "juror-a.webvtt"))
juror_b = webvtt.read(os.path.join("captions", "juror-b.webvtt"))
juror_c = webvtt.read(os.path.join("captions", "juror-c.webvtt"))
jury_foreman = webvtt.read(os.path.join("captions", "jury-foreman.webvtt"))

juror_a_captions = juror_a.captions
juror_b_captions = juror_b.captions
juror_c_captions = juror_c.captions
jury_foreman_captions = jury_foreman.captions


merged_captions: Iterable[Caption] = sorted(
    juror_a_captions + juror_b_captions + juror_c_captions + jury_foreman_captions,
    key=lambda x: x.start_in_seconds,
)

merged_webvtt = webvtt.WebVTT("merged_captions.webvtt", captions=merged_captions)

with open(os.path.join("captions", "merged_captions.webvtt"), "w+") as f:
    merged_webvtt.write(f)
