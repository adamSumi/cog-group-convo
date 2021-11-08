import os
import webvtt
from collections import deque
from typing import Iterable, List, Dict, Tuple
from pprint import pprint
import json

from webvtt.structures import Caption
from common import JurorId

juror_a = webvtt.read(os.path.join("captions", "juror-a.webvtt"))
juror_b = webvtt.read(os.path.join("captions", "juror-b.webvtt"))
juror_c = webvtt.read(os.path.join("captions", "juror-c.webvtt"))
jury_foreman = webvtt.read(os.path.join("captions", "jury-foreman.webvtt"))

juror_a_captions = [("juror-a", caption) for caption in juror_a.captions]
juror_b_captions = [("juror-b", caption) for caption in juror_b.captions]
juror_c_captions = [("juror-c", caption) for caption in juror_c.captions]
jury_foreman_captions = [("jury-foreman", caption) for caption in jury_foreman.captions]


merged_captions: Iterable[Tuple[JurorId, Caption]] = sorted(
    juror_a_captions + juror_b_captions + juror_c_captions + jury_foreman_captions,
    key=lambda x: x[1].start_in_seconds,
)

for _, caption in merged_captions:
    print(caption.start_in_seconds, caption.text)

i = 0
transformed_captions: List[Dict[str, str]] = []
merged_captions: List[Tuple[JurorId, Caption]] = [
    (juror_id, caption) for juror_id, caption in merged_captions if caption.text.strip()
]
for i, (juror_id, caption) in enumerate(merged_captions):
    chunks = list(filter(None, caption.text.strip().split(" ")))
    total_caption_duration_in_millis = (
        caption.end_in_seconds - caption.start_in_seconds
    ) * 1000
    duration_per_chunk = total_caption_duration_in_millis / len(chunks)
    for j, chunk in enumerate(chunks):
        transformed_captions.append(
            {
                "text": chunk,
                "message_id": i,
                "chunk_id": j,
                "delay": caption.start_in_seconds * 1000 + (j * duration_per_chunk),
                "speaker_id": juror_id,
            }
        )

with open(os.path.join("captions", "merged_captions.json"), "w+") as f:
    json.dump(transformed_captions, f)
