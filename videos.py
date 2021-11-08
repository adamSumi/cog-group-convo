import vlc
import time
import multiprocessing


def play_video(
    video_path: str,
    ready_to_start_playing: multiprocessing.Event,
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
    while not ready_to_start_playing.is_set():
        pass
    media_player.set_time(0)
    media_player.play()
    time.sleep(0.5)
    while media_player.is_playing():
        pass
