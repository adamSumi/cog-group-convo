ffmpeg -i juror-a.mp4 -ss 00:00:00 -to 00:02:30 -c copy juror-a.1.mp4
ffmpeg -i juror-a.mp4 -ss 00:02:30 -to 00:05:00 -c copy juror-a.2.mp4
ffmpeg -i juror-a.mp4 -ss 00:05:00 -to 00:08:30 -c copy juror-a.3.mp4
ffmpeg -i juror-a.mp4 -ss 00:08:30 -to 00:10:00 -c copy juror-a.4.mp4

ffmpeg -i juror-b.mp4 -ss 00:00:00 -to 00:02:30 -c copy juror-b.1.mp4
ffmpeg -i juror-b.mp4 -ss 00:02:30 -to 00:05:00 -c copy juror-b.2.mp4
ffmpeg -i juror-b.mp4 -ss 00:05:00 -to 00:08:30 -c copy juror-b.3.mp4
ffmpeg -i juror-b.mp4 -ss 00:08:30 -to 00:10:00 -c copy juror-b.4.mp4

ffmpeg -i juror-c.mp4 -ss 00:00:00 -to 00:02:30 -c copy juror-c.1.mp4
ffmpeg -i juror-c.mp4 -ss 00:02:30 -to 00:05:00 -c copy juror-c.2.mp4
ffmpeg -i juror-c.mp4 -ss 00:05:00 -to 00:08:30 -c copy juror-c.3.mp4
ffmpeg -i juror-c.mp4 -ss 00:08:30 -to 00:10:00 -c copy juror-c.4.mp4

ffmpeg -i jury-foreman.mp4 -ss 00:00:00 -to 00:02:30 -c copy jury-foreman.1.mp4
ffmpeg -i jury-foreman.mp4 -ss 00:02:30 -to 00:05:00 -c copy jury-foreman.2.mp4
ffmpeg -i jury-foreman.mp4 -ss 00:05:00 -to 00:08:30 -c copy jury-foreman.3.mp4
ffmpeg -i jury-foreman.mp4 -ss 00:08:30 -to 00:10:00 -c copy jury-foreman.4.mp4