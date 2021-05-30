example output: https://www.youtube.com/watch?v=UbaBI-XxGbo

everything is pretty much hardcoded, I don't expect to reuse it

change the code according to your needs first

## Info
- Reads and Writes only **PNG**
- Input goes into **in/** with format **img_#.png**
- Output comes out in **out/** with format **img_#.png**
- Only works on **Black/White** Images 
- Used with PNG Type: **RGB/RGBA** (others could break)
- Specify the frame range in main.cpp
- **Warning** slow

# Credits
This uses [stb_image, stb_image_write](https://github.com/nothings/stb), and [thread-pool](https://github.com/bshoshany/thread-pool)
