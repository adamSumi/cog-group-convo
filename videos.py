import vlc
import time
import multiprocessing
import os
from typing import Any, Optional, List

from common import RenderingMethod


def get_audio_devices() -> List:
    instance: vlc.Instance = vlc.Instance()
    player = instance.media_player_new()

    output_devices = []
    mods = player.audio_output_device_enum()
    if mods:
        mod = mods
        while mod:
            mod = mod.contents
            output_devices.append(mod.device)
            mod = mod.next
    vlc.libvlc_audio_output_device_list_release(mods)
    return output_devices


def play_video(
    video_path: str,
    ready_to_start_playing: multiprocessing.Event,
    caption_url: str,
    should_show_subtitles: multiprocessing.Event,
    device_id: bytes,
    is_muted: bool = True,
):
    instance: vlc.Instance = vlc.Instance()
    media_player: vlc.MediaPlayer = instance.media_player_new()
    media_player.audio_set_mute(True)
    media_player.audio_output_device_set(None, device_id)
    media: vlc.Media = instance.media_new(video_path)
    media_player.set_media(media)
    media_player.play()
    time.sleep(0.5)
    media_player.pause()
    ready_to_start_playing.wait()
    media_player.set_time(0)
    media_player.audio_set_mute(is_muted)
    media_player.play()
    time.sleep(0.5)
    while media_player.is_playing():
        if should_show_subtitles.is_set():
            if media_player.video_get_spu_count() == 0:
                # caption_url must be in the form of "file://"
                media_player.add_slave(vlc.MediaSlaveType(0), caption_url, True)
        else:
            if media_player.video_get_spu_count() > 0:
                media_player.get_media().slaves_clear()


if __name__ == "__main__":
    ready_to_start_playing = multiprocessing.Event()
    ready_to_start_playing.set()
    should_show_subtitles = multiprocessing.Event()
    should_show_subtitles.set()
    play_video(
        "videos/juror-a.mp4",
        ready_to_start_playing,
        RenderingMethod.MONITOR_ONLY,
        "captions/juror-a.webvtt",
        should_show_subtitles,
        False,
    )
