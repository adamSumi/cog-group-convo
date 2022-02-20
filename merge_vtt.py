import os
import webvtt
from typing import Iterable
from pprint import pprint

from webvtt.structures import Caption

for i in range(1, 5):

    juror_a = webvtt.read(os.path.join("captions", f"juror-a.{i}.vtt"))
    juror_b = webvtt.read(os.path.join("captions", f"juror-b.{i}.vtt"))
    juror_c = webvtt.read(os.path.join("captions", f"juror-c.{i}.vtt"))
    jury_foreman = webvtt.read(os.path.join("captions", f"jury-foreman.{i}.vtt"))
    juror_a_captions = juror_a.captions
    juror_b_captions = juror_b.captions
    juror_c_captions = juror_c.captions
    jury_foreman_captions = jury_foreman.captions

    merged_captions: Iterable[Caption] = sorted(
        juror_a_captions + juror_b_captions + juror_c_captions + jury_foreman_captions,
        key=lambda x: x.start_in_seconds,
    )

    merged_webvtt = webvtt.WebVTT(
        os.path.join("captions", f"merged_captions.{i}.vtt"),
        captions=merged_captions,
    )
    merged_webvtt.save()
