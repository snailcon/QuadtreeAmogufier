#ifndef IMAGE_H
#define IMAGE_H
#endif

#include <stdint.h>
#include <iostream>
#include <cstring>
#include <string>
#include <math.h>
#include <vector>

struct Image {
    uint8_t* data = NULL;
    size_t size = 0;
    int w;
    int h;
    int channels;

    Image(const char* filename);
    Image(int w, int h, int channels);
    Image(const Image& img);
    ~Image();

    bool read(const char* filename);
    bool write(const char* filename);

    Image& colorMask(float r, float g, float b);
    Image colorMaskNew(float r, float g, float b);
    Image& overlay(const Image& source, int x, int y);
    Image& resizeFast(uint16_t rw, uint16_t rh); // nearest neighbor
    Image resizeFastNew(uint16_t rw, uint16_t rh);
    Image cropNew(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch);

    Image& rect(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch, uint8_t r, uint8_t b, uint8_t g);
    Image& rectOutline(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch, uint8_t r, uint8_t b, uint8_t g);

    Image& rect(uint8_t r, uint8_t b, uint8_t g);
    Image& rectOutline(uint8_t r, uint8_t b, uint8_t g);

    Image quadifyFrame(int i);
    void subdivide(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, int i, Image& frame, Image& amogus, Image& amogus_eye);

    int subdivideCheck(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh);
};
