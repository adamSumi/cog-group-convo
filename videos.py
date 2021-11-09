import vlc
import time
import multiprocessing
import os

from common import RenderingMethod


def play_video(
    video_path: str,
    ready_to_start_playing: multiprocessing.Event,
    rendering_method: RenderingMethod,
    caption_url: str = "",
    is_muted: bool = True,
):
    instance: vlc.Instance = vlc.Instance()
    media_player: vlc.MediaPlayer = instance.media_player_new()
    media: vlc.Media = instance.media_new(video_path)
    media_player.set_media(media)
    media_player.audio_set_mute(is_muted)
    print("Rendering method =", rendering_method)
    if rendering_method in (
        RenderingMethod.MONITOR_ONLY,
        RenderingMethod.MONITOR_AND_GLOBAL,
        RenderingMethod.MONITOR_AND_GLOBAL_WITH_DIRECTION_INDICATORS,
    ):
        # caption_url must be in the form of "file://"
        media_player.add_slave(vlc.MediaSlaveType(0), caption_url, True)
    media_player.play()
    time.sleep(0.5)
    media_player.pause()
    while not ready_to_start_playing.is_set():
        pass
    media_player.set_time(0)
    media_player.play()
    time.sleep(0.5)
    while media_player.is_playing():
        pass
