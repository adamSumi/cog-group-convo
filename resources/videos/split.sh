ffmpeg -i main.mp4 -ss 00:00:00 -t 00:02:30 -c copy main.1.mp4
ffmpeg -i main.mp4 -ss 00:02:30 -t 00:02:30 -c copy main.2.mp4
ffmpeg -i main.mp4 -ss 00:05:00 -t 00:02:30 -c copy main.3.mp4
ffmpeg -i main.mp4 -ss 00:07:30 -t 00:02:30 -c copy main.4.mp4