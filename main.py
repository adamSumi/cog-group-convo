import os
import random
from typing import List, Union

import cv2

from captions import CAPTIONS

VIDEO_DIRECTORY = "videos"
JUROR_A_IDLE_VIDEOS = os.path.join(VIDEO_DIRECTORY, "idle", "juror-a")
JUROR_B_IDLE_VIDEOS = os.path.join(VIDEO_DIRECTORY, "idle", "juror-b")
JUROR_C_IDLE_VIDEOS = os.path.join(VIDEO_DIRECTORY, "idle", "juror-c")
JURY_FOREMAN_IDLE_VIDEOS = os.path.join(VIDEO_DIRECTORY, "idle", "jury-foreman")

JUROR_TO_IDLE_VIDEOS = {
    "juror-a": JUROR_A_IDLE_VIDEOS,
    "juror-b": JUROR_B_IDLE_VIDEOS,
    "juror-c": JUROR_C_IDLE_VIDEOS,
    "jury-foreman": JURY_FOREMAN_IDLE_VIDEOS,
}
WINDOW_TITLES = ["Juror A", "Juror B", "Juror C", "Jury Foreman"]

JUROR_TO_INDEX = {"juror-a": 0, "juror-b": 1, "juror-c": 2, "jury-foreman": 3}

INDEX_TO_JUROR = {value: key for key, value in JUROR_TO_INDEX.items()}


def get_random_video_from_directory(video_directory: str) -> str:
    return os.path.join(video_directory, random.choice(os.listdir(video_directory)))


def get_active_video_from_index(index: int) -> str:
    return os.path.join(VIDEO_DIRECTORY, f"{index+1}.mp4")


def main():
    counter = 0
    current_caption_info = CAPTIONS[counter]
    initial_video_paths: List[str] = [
        get_random_video_from_directory(JUROR_A_IDLE_VIDEOS),
        get_random_video_from_directory(JUROR_B_IDLE_VIDEOS),
        get_random_video_from_directory(JUROR_C_IDLE_VIDEOS),
        os.path.join(VIDEO_DIRECTORY, f"{counter + 1}.mp4"),
    ]

    playing_videos = [
        cv2.VideoCapture(video_path) for video_path in initial_video_paths
    ]

    frames = [None] * len(initial_video_paths)
    ret = [None] * len(initial_video_paths)
    print(current_caption_info["text"])
    while True:
        for i, video in enumerate(playing_videos):
            if video is not None:
                is_playing, frame = video.read()
                ret[i] = is_playing
                frames[i] = frame
                if not is_playing:
                    juror = INDEX_TO_JUROR[i]
                    # Replace active speaker's current video with an idle video.
                    random_idle_video_path = get_random_video_from_directory(
                        JUROR_TO_IDLE_VIDEOS[juror]
                    )
                    video.release()
                    playing_videos[i] = cv2.VideoCapture(random_idle_video_path)

                    # Active speaker video has finished playing
                    if current_caption_info["id"] == juror:
                        # Swapping next speaker's idle video out with an active one.
                        counter += 1
                        if counter >= len(CAPTIONS):
                            break
                        current_caption_info = CAPTIONS[counter]
                        new_speaker_id = current_caption_info["id"]
                        new_speaker_index = JUROR_TO_INDEX[new_speaker_id]

                        # The speaker we're about to switch to is currently playing an idle video, let's release that video from memory
                        new_speaker_idle_video = playing_videos[new_speaker_index]
                        new_speaker_idle_video.release()

                        # Now let's replace that video with the active video.
                        new_speaker_active_video_path = get_active_video_from_index(counter)
                        new_speaker_active_video = cv2.VideoCapture(new_speaker_active_video_path)
                        playing_videos[new_speaker_index] = new_speaker_active_video

                        # Debugging print statement
                        print(current_caption_info["text"])

                        

        for i, f in enumerate(frames):
            if ret[i] is True:
                cv2.imshow(WINDOW_TITLES[i], f)

        if cv2.waitKey(16) & 0xFF == ord("q"):
            break

    for video in playing_videos:
        if video is not None:
            video.release()

    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
