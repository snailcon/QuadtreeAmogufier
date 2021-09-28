#ifndef IMAGE_H
#define IMAGE_H
#endif

#include <stdint.h>
#include <iostream>
#include <cstring>
#include <string>
#include <math.h>
#include <vector>
#include <map>

struct Image {
    std::vector<uint8_t> data;
    size_t size = 0;
    int w;
    int h;
    int channels;

    Image();
    Image(const char* filename);
    Image(int w, int h, int channels);
    Image(const Image& img);

    uint8_t &operator()(int x, int y, int c) { return pixel(x, y)[c]; }
    const uint8_t &operator()(int x, int y, int c) const { return pixel(x, y)[c]; }

    uint8_t *pixel(int x, int y) { return data.data() + (x + y * w) * channels; }
    const uint8_t *pixel(int x, int y) const { return data.data() + (x + y * w) * channels; }

    bool read(const char* filename);
    bool write(const char* filename) const;

    Image& colorMask(float r, float g, float b);
    Image &colorMask(uint8_t r, uint8_t g, uint8_t b);
    Image colorMaskNew(float r, float g, float b) const;
    Image colorMaskNew(uint8_t r, uint8_t g, uint8_t b) const;
    Image &overlay(const Image &source, int x, int y);
    Image resizeFastNew(uint16_t rw, uint16_t rh) const;
    Image cropNew(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch) const;

    Image quadifyFrameBW(std::map<std::pair<int, int>, Image> &resizedAmogi) const;
    void subdivideBW(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, Image &frame,
                     std::map<std::pair<int, int>, Image> &resizedAmogi) const;
    std::tuple<bool, uint8_t> subdivideCheckBW(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh) const;

    Image quadifyFrameRGB(std::map<std::pair<int, int>, Image> &resizedAmogi) const;
    void subdivideRGB(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, Image &frameRGB,
                      std::map<std::pair<int, int>, Image> &resizedAmogi) const;
    std::tuple<bool, int, int, int> subdivideCheckRGB(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh) const;

    std::map<std::pair<int, int>, Image> preloadResized(int sw, int sh) const;
    void subdivideValues(int sx, int sy, int sw, int sh, std::map<std::pair<int, int>, Image> &image_map) const;
};