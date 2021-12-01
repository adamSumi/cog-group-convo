ffmpeg -i juror-a.mp4 -ss 00:00:00 -t 00:02:30 juror-a.1.mp4
ffmpeg -i juror-a.mp4 -ss 00:02:30 -t 00:02:30 juror-a.2.mp4
ffmpeg -i juror-a.mp4 -ss 00:05:00 -t 00:02:30 juror-a.3.mp4
ffmpeg -i juror-a.mp4 -ss 00:07:30 -t 00:02:30 juror-a.4.mp4

ffmpeg -i juror-b.mp4 -ss 00:00:00 -t 00:02:30 juror-b.1.mp4
ffmpeg -i juror-b.mp4 -ss 00:02:30 -t 00:02:30 juror-b.2.mp4
ffmpeg -i juror-b.mp4 -ss 00:05:00 -t 00:02:30 juror-b.3.mp4
ffmpeg -i juror-b.mp4 -ss 00:07:30 -t 00:02:30 juror-b.4.mp4

ffmpeg -i juror-c.mp4 -ss 00:00:00 -t 00:02:30 juror-c.1.mp4
ffmpeg -i juror-c.mp4 -ss 00:02:30 -t 00:02:30 juror-c.2.mp4
ffmpeg -i juror-c.mp4 -ss 00:05:00 -t 00:02:30 juror-c.3.mp4
ffmpeg -i juror-c.mp4 -ss 00:07:30 -t 00:02:30 juror-c.4.mp4

ffmpeg -i jury-foreman.mp4 -ss 00:00:00 -t 00:02:30 jury-foreman.1.mp4
ffmpeg -i jury-foreman.mp4 -ss 00:02:30 -t 00:02:30 jury-foreman.2.mp4
ffmpeg -i jury-foreman.mp4 -ss 00:05:00 -t 00:02:30 jury-foreman.3.mp4
ffmpeg -i jury-foreman.mp4 -ss 00:07:30 -t 00:02:30 jury-foreman.4.mp4