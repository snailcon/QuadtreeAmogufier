#include <vector>
#include <string>

#include "Image.h"

void createVideoFrames();

int main()
{
    createVideoFrames();

    std::cout<<"\n\nDone"<<std::endl;
    return 0;
}

void createVideoFrames() {
    for (int i = 0; i < 6572; i ++) { // manually change frame numbers
        std::cout<<i<<"\n";
        std::string frame_name("in/img_" + std::to_string(i) + ".png");
        Image frame(frame_name.c_str());
        Image frame_done = frame.quadifyFrame(i);
        std::string save_loc("out/img_" + std::to_string(i) + ".png");
        frame_done.write(save_loc.c_str());
    }
}