#include <map>
#include <string>
#include <vector>

#include "Image.h"
#include "lib/thread_pool.hpp"

void workBW(int i, int index, std::vector<std::map<std::pair<int, int>, Image>> &preloadedResized);
void workCol(int i, int index, std::vector<std::map<std::pair<int, int>, Image>> &preloadedResized);

void createVideoFramesBW(int start, int end, int repeatFrames);
void createVideoFramesCol(int start, int end, int repeatFrames);

void showUsage() {
    std::cout << "Usage: [?.exe] [BW | Col] [Start] [End] (SFRC)\n"
              << "BW | Col:   Black and White or Colored Image Sequence\n"
              << "Start:      Frame to start on (int)\n"
              << "End:        Frame to end on (int)\n"
              << "SFRC:       How often to repeat Sprite frames (optional, default 2)" << std::endl;
}

int main(int argc, char *argv[]) {
    std::string type;
    int start, end, repeatFrames;
    if (argc < 4) {
        showUsage();
        return 0;
    } else {
        type = argv[1];
        start = std::stoi(argv[2]);
        end = std::stoi(argv[3]);
        repeatFrames = 2;
    }
    if (argc > 4) {
        repeatFrames = std::stoi(argv[4]);
    }

    if (type == "BW") {
        createVideoFramesBW(start, end, repeatFrames);
    } else if (type == "Col") {
        createVideoFramesCol(start, end, repeatFrames);
    } else {
        showUsage();
        return 0;
    }

    std::cout << "\n\nDone" << std::endl;
    return 0;
}

void createVideoFramesBW(int start, int end, int repeatFrames) {

    std::vector<std::map<std::pair<int, int>, Image>> preloadedResized;
    int width;
    int height;
    std::string first_name("in/img_" + std::to_string(start) + ".png");
    Image first_frame(first_name.c_str());
    width = first_frame.w;
    height = first_frame.h;

    for (int i = 0; i < 6; i++) {
        std::string amogus_name("res/" + std::to_string(i) + ".png");
        Image amogus(amogus_name.c_str());

        preloadedResized.push_back(amogus.preloadResized(width, height));
    }

    thread_pool pool;

    for (int i = start; i <= end; i++) {
        int index = (i % (6 * repeatFrames)) / repeatFrames;
        pool.submit(workBW, i, index, std::ref(preloadedResized));
    }

    pool.wait_for_tasks();
}

void workBW(int i, int index, std::vector<std::map<std::pair<int, int>, Image>> &preloadedResized) {
    std::string frame_name("in/img_" + std::to_string(i) + ".png");
    Image frame(frame_name.c_str());
    Image frame_done = frame.quadifyFrameBW(preloadedResized.at(index));
    std::string save_loc("out/img_" + std::to_string(i) + ".png");
    frame_done.write(save_loc.c_str());
    std::cout << i << "\n";
}

void createVideoFramesCol(int start, int end, int repeatFrames) {

    std::vector<std::map<std::pair<int, int>, Image>> preloadedResized;
    int width;
    int height;
    std::string first_name("in/img_" + std::to_string(start) + ".png");
    Image first_frame(first_name.c_str());
    width = first_frame.w;
    height = first_frame.h;

    for (int i = 0; i < 6; i++) {
        std::string amogus_name("res/" + std::to_string(i) + ".png");
        Image amogus(amogus_name.c_str());

        preloadedResized.push_back(amogus.preloadResized(width, height));
    }

    thread_pool pool;

    for (int i = start; i <= end; i++) {
        int index = (i % (6 * repeatFrames)) / repeatFrames;
        pool.submit(workCol, i, index, std::ref(preloadedResized));
    }

    pool.wait_for_tasks();
}

void workCol(int i, int index, std::vector<std::map<std::pair<int, int>, Image>> &preloadedResized) {
    std::string frame_name("in/img_" + std::to_string(i) + ".png");
    Image frame(frame_name.c_str());
    Image frame_done = frame.quadifyFrameRGB(preloadedResized.at(index));
    std::string save_loc("out/img_" + std::to_string(i) + ".png");
    frame_done.write(save_loc.c_str());
    std::cout << i << "\n";
}