example output BW: https://www.youtube.com/watch?v=UbaBI-XxGbo

do whatever you want with this

## What is this
This takes an image sequence and converts it to a quadtree structured image sequence with a gif/sprite of your choice (place it in res/ and update it in code).

you can set the max and min square size in the subdivide methods (functions?)

## Info
- Reads and Writes **PNG**
- Input goes into **in/** with format **img_#.png**
- Output comes out in **out/** with format **img_#.png**
- Not that slow anymore
- Usage Instructions in Code / when running without args
- Requires C++17 features enabled (thread-pool)

# Credits
[stb_image / stb_image_write](https://github.com/nothings/stb)

[thread-pool](https://github.com/bshoshany/thread-pool) - by recommendation of [ramidzkh](https://github.com/ramidzkh)