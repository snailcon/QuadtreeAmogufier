#include <filesystem>
#include "lib/thread_pool.hpp"
#include "Image.h"

void work(int i, const std::filesystem::path &frame_name, const std::filesystem::path &save_loc);

int main() {
    thread_pool pool;

    int index = 0;

    auto in = std::filesystem::path("in");
    auto out = std::filesystem::path("out");

    for (const auto &entry : std::filesystem::directory_iterator(in)) {
        auto path = std::filesystem::relative(entry);
        pool.submit(work, index++, path, out / relative(path, in));
    }

    pool.wait_for_tasks();
    std::cout << "\n\nDone" << std::endl;

    return 0;
}

void work(int i, const std::filesystem::path &frame_name, const std::filesystem::path &save_loc) {
    std::cout << i << std::endl;

    Image frame(frame_name.string().c_str());
    Image frame_done = frame.quadifyFrame(i);

    frame_done.write(save_loc.string().c_str());
}
