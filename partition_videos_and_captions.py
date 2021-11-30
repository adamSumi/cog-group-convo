import os
import webvtt
import subprocess
import datetime
import time

from webvtt.structures import Caption

PARTITIONS = (0, 150, 300, 450, 600)


def shift_caption(caption: Caption, offset: int) -> Caption:
    new_start = time.strftime(
        "%H:%M:%S.000", time.gmtime(caption.start_in_seconds - offset)
    )
    new_end = time.strftime(
        "%H:%M:%S.000", time.gmtime(caption.end_in_seconds - offset)
    )
    caption.start = new_start
    caption.end = new_end
    return caption


def main():
    juror_a_webvtt: webvtt.WebVTT = webvtt.read(os.path.join("captions", "juror-a.vtt"))
    juror_b_webvtt: webvtt.WebVTT = webvtt.read(os.path.join("captions", "juror-b.vtt"))
    juror_c_webvtt: webvtt.WebVTT = webvtt.read(os.path.join("captions", "juror-c.vtt"))
    jury_foreman_webvtt: webvtt.WebVTT = webvtt.read(
        os.path.join("captions", "jury-foreman.vtt")
    )
    for i in range(1, len(PARTITIONS)):
        start_timestamp = time.strftime("%H:%M:%S", time.gmtime(PARTITIONS[i - 1]))
        end_timestamp = time.strftime("%H:%M:%S", time.gmtime(PARTITIONS[i]))
        juror_a_partitioned_captions = [
            shift_caption(caption, PARTITIONS[i - 1])
            for caption in juror_a_webvtt.captions
            if caption.start_in_seconds >= PARTITIONS[i - 1]
            and caption.end_in_seconds < PARTITIONS[i]
        ]
        juror_a_partition_webvtt = webvtt.WebVTT(
            file=f"captions/juror-a.{i}.vtt", captions=juror_a_partitioned_captions
        )
        juror_a_partition_webvtt.save()

        juror_b_partitioned_captions = [
            shift_caption(caption, PARTITIONS[i - 1])
            for caption in juror_b_webvtt.captions
            if caption.start_in_seconds >= PARTITIONS[i - 1]
            and caption.end_in_seconds < PARTITIONS[i]
        ]
        juror_b_partition_webvtt = webvtt.WebVTT(
            file=f"captions/juror-b.{i}.vtt", captions=juror_b_partitioned_captions
        )
        juror_b_partition_webvtt.save()

        juror_c_partitioned_captions = [
            shift_caption(caption, PARTITIONS[i - 1])
            for caption in juror_c_webvtt.captions
            if caption.start_in_seconds >= PARTITIONS[i - 1]
            and caption.end_in_seconds < PARTITIONS[i]
        ]
        juror_c_partition_webvtt = webvtt.WebVTT(
            file=f"captions/juror-c.{i}.vtt", captions=juror_c_partitioned_captions
        )
        juror_c_partition_webvtt.save()

        jury_foreman_partitioned_captions = [
            shift_caption(caption, PARTITIONS[i - 1])
            for caption in jury_foreman_webvtt.captions
            if caption.start_in_seconds >= PARTITIONS[i - 1]
            and caption.end_in_seconds < PARTITIONS[i]
        ]
        jury_foreman_partition_webvtt = webvtt.WebVTT(
            file=f"captions/jury-foreman.{i}.vtt",
            captions=jury_foreman_partitioned_captions,
        )
        jury_foreman_partition_webvtt.save()

        subprocess.call(
            f"ffmpeg -err_detect ignore_err -i videos/juror-a.mp4 -ss {start_timestamp} -to {end_timestamp} -c copy videos/juror-a.{i}.mp4",
            shell=True,
        )

        subprocess.call(
            f"ffmpeg -err_detect ignore_err -i videos/juror-b.mp4 -ss {PARTITIONS[i-1]} -to {PARTITIONS[i]} -c copy videos/juror-b.{i}.mp4",
            shell=True,
        )

        subprocess.call(
            f"ffmpeg -err_detect ignore_err -i videos/juror-c.mp4 -ss {PARTITIONS[i-1]} -to {PARTITIONS[i]} -c copy videos/juror-c.{i}.mp4",
            shell=True,
        )
        subprocess.call(
            f"ffmpeg -err_detect ignore_err -i videos/jury-foreman.mp4 -ss {PARTITIONS[i-1]} -to {PARTITIONS[i]} -c copy videos/jury-foreman.{i}.mp4",
            shell=True,
        )


if __name__ == "__main__":
    main()
