#include <string>
#include "lib/thread_pool.hpp"
#include "Image.h"

void work(int i);

int main() {
    thread_pool pool;

    for (int i = 0; i < 6572; i++) { // manually change frame numbers
        pool.submit(work, i);
    }

    pool.wait_for_tasks();
    std::cout << "\n\nDone" << std::endl;

    return 0;
}

void work(int i) {
    std::cout << i << std::endl;
    std::string frame_name("in/img_" + std::to_string(i) + ".png");
    std::string save_loc("out/img_" + std::to_string(i) + ".png");

    Image frame(frame_name.c_str());
    Image frame_done = frame.quadifyFrame(i);

    frame_done.write(save_loc.c_str());
}
