import vlc
import time
import multiprocessing
import os
from typing import Optional

from common import RenderingMethod


def play_video(
    video_path: str,
    ready_to_start_playing: multiprocessing.Event,
    rendering_method: RenderingMethod,
    caption_url: str,
    should_show_subtitles: multiprocessing.Event,
    is_muted: bool = True,
):
    instance: vlc.Instance = vlc.Instance()
    media_player: vlc.MediaPlayer = instance.media_player_new()
    media: vlc.Media = instance.media_new(video_path)
    media_player.set_media(media)
    media_player.audio_set_mute(is_muted)
    media_player.play()
    time.sleep(0.5)
    media_player.pause()
    ready_to_start_playing.wait()
    media_player.set_time(0)
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
