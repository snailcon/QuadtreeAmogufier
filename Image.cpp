#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Image.h"

#include "lib/stb_image.h"
#include "lib/stb_image_write.h"

namespace {
template <class T> T rescale(T &val, double s) {
    T old = val;
    val = static_cast<T>(old * s);
    return old;
}

template <class T> T rescale(T &val, uint8_t s) {
    T old = val;
    val = old * s / 255;
    return old;
}

template <class T> T bound(double x) {
    if (x < std::numeric_limits<T>::min())
        return std::numeric_limits<T>::min();
    if (x > std::numeric_limits<T>::max())
        return std::numeric_limits<T>::max();
    return static_cast<T>(std::round(x));
}
} // namespace

Image::Image() : w(100), h(100), channels(3) {
    size = w * h * channels;
    data = std::vector<uint8_t>(size);
}

Image::Image(const char *filename) {
    if (read(filename)) {
        // std::cout<<"Read "<<filename<<" Width: "<<w<<" Height: "<<h<<" Channels: "<<channels<<std::endl;
        size = w * h * channels;
    } else {
        std::cout << "Failed to read " << filename << std::endl;
    }
}

Image::Image(int w, int h, int channels) : w(w), h(h), channels(channels) {
    size = w * h * channels;
    data = std::vector<uint8_t>(size);
}

Image::Image(const Image &img) : w(img.w), h(img.h), channels(img.channels), data(img.data) { size = w * h * channels; }

// uint8_t &Image::operator()(int x, int y, int c) { return data[(x + y * w) * channels + c]; }
// const uint8_t &Image::operator()(int x, int y, int c) const { return data[(x + y * w) * channels + c]; }

// uint8_t *Image::pixel(int x, int y) { return &(*this)(x, y, 0); }
// const uint8_t *Image::pixel(int x, int y) const { return &(*this)(x, y, 0); }

bool Image::read(const char *filename) {
    uint8_t *temp = stbi_load(filename, &w, &h, &channels, 0);
    size = w * h * channels;
    // data.clear();
    data.insert(data.end(), &temp[0], &temp[size]);
    stbi_image_free(temp);
    return true;
}

bool Image::write(const char *filename) const {
    int success;
    success = stbi_write_png(filename, w, h, channels, data.data(), w * channels);
    return success != 0;
}

Image &Image::colorMask(float r, float g, float b) {
    assert(channels == 3);
    for (int i = 0; i < data.size(); i += channels) {
        rescale(data[i], r);
        rescale(data[i + 1], g);
        rescale(data[i + 2], b);
    }
    return *this;
}

Image &Image::colorMask(uint8_t r, uint8_t g, uint8_t b) {
    assert(channels == 3);
    for (int i = 0; i < data.size(); i += channels) {
        rescale(data[i], r);
        rescale(data[i + 1], g);
        rescale(data[i + 2], b);
    }
    return *this;
}

Image Image::colorMaskNew(float r, float g, float b) const {
    Image new_version = *this;
    new_version.colorMask(r, g, b);
    return new_version;
}

Image Image::colorMaskNew(uint8_t r, uint8_t g, uint8_t b) const {
    Image new_version = *this;
    new_version.colorMask(r, g, b);
    return new_version;
}

Image &Image::overlay(const Image &source, int x, int y) {

    for (int sy = std::max(0, -y); sy < source.h; sy++) {
        if (sy + y >= h)
            break;
        for (int sx = std::max(0, -x); sx < source.w; sx++) {
            if (sx + x >= w)
                break;

            const uint8_t *srcPixel = source.pixel(sx, sy);
            uint8_t *dstPixel = pixel(sx + x, sy + y);
            float srcAlpha = source.channels < 4 ? 1 : srcPixel[3] / 255.f;
            float dstAlpha = channels < 4 ? 1 : dstPixel[3] / 255.f;

            if (srcAlpha > .99 && dstAlpha > .99) {
                std::copy_n(srcPixel, channels, dstPixel);
            } else {
                float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);
                if (outAlpha < .01) {
                    std::fill_n(dstPixel, channels, 0);
                } else {
                    for (int channel = 0; channel < channels; channel++) {
                        dstPixel[channel] = bound<uint8_t>((srcPixel[channel] / 255.f * srcAlpha +
                                                            dstPixel[channel] / 255.f * dstAlpha * (1 - srcAlpha)) /
                                                           outAlpha * 255.f);
                    }
                    if (channels > 3)
                        dstPixel[3] = bound<uint8_t>(outAlpha * 255.f);
                }
            }
        }
    }

    return *this;
}

Image Image::resizeFastNew(uint16_t rw, uint16_t rh) const {
    Image resizedImage(rw, rh, channels);
    double x_ratio = w / (double)rw;
    double y_ratio = h / (double)rh;
    for (int y = 0; y < rh; y++) {
        for (int x = 0; x < rw; x++) {
            int rx = static_cast<int>(x * x_ratio);
            int ry = static_cast<int>(y * y_ratio);
            std::copy_n(pixel(rx, ry), channels, resizedImage.pixel(x, y));
        }
    }
    return resizedImage;
}

Image Image::cropNew(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch) const {

    Image croppedImage(cw, ch, channels);

    for (uint16_t y = 0; y < ch; y++) {
        if (y + cy >= h)
            break;
        for (uint16_t x = 0; x < cw; x++) {
            if (x + cx >= w)
                break;
            std::copy_n(pixel(x + cx, y + cy), channels, croppedImage.pixel(x, y));
        }
    }

    return croppedImage;
}

Image Image::quadifyFrameBW(std::map<std::pair<int, int>, Image> &resizedAmogi) const {
    Image frame(w, h, 3);

    subdivideBW(0, 0, w, h, frame, resizedAmogi);

    return frame;
}

// sw: subdivided x | sy subdivided y
// sw: subdivided width | sh subdivided height
void Image::subdivideBW(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, Image &frame,
                        std::map<std::pair<int, int>, Image> &resizedAmogi) const {

    auto [subdivide, val] = subdivideCheckBW(sx, sy, sw, sh);

    if (subdivide && sw > 16 && sh > 16) {
        uint16_t sw_l = sw / 2;
        uint16_t sw_r = (sw + 1) / 2;
        uint16_t sh_t = sh / 2;
        uint16_t sh_b = (sh + 1) / 2;
        subdivideBW(sx, sy, sw_l, sh_t, frame, resizedAmogi);
        subdivideBW(sx + sw_r, sy, sw_l, sh_t, frame, resizedAmogi);
        subdivideBW(sx, sy + sh_b, sw_l, sh_t, frame, resizedAmogi);
        subdivideBW(sx + sw_r, sy + sh_b, sw_l, sh_t, frame, resizedAmogi);
    } else {
        if (val <= 20)
            return;
        frame.overlay(resizedAmogi[std::make_pair(sw, sh)].colorMaskNew(val / 255.f, val / 255.f, val / 255.f), sx, sy);
    }
}

std::tuple<bool, uint8_t> Image::subdivideCheckBW(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh) const {
    double sum = 0;
    uint8_t min = std::numeric_limits<uint8_t>::max();
    uint8_t max = std::numeric_limits<uint8_t>::min();

    for (uint16_t y = sy; y < sh + sy; y++) {
        for (uint16_t x = sx; x < sw + sx; x++) {
            uint8_t p = pixel(x, y)[0];
            min = std::min(min, p);
            max = std::max(max, p);
            sum += p;
        }
    }

    return {max - min > 4, bound<uint8_t>(sum / (sh * sw))};
}

Image Image::quadifyFrameRGB(std::map<std::pair<int, int>, Image> &resizedAmogi) const {
    Image frameRGB(w, h, 3);

    subdivideRGB(0, 0, w, h, frameRGB, resizedAmogi);

    return frameRGB;
}

void Image::subdivideRGB(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, Image &frameRGB,
                         std::map<std::pair<int, int>, Image> &resizedAmogi) const {

    std::tuple<bool, int, int, int> check = subdivideCheckRGB(sx, sy, sw, sh);
    bool quad = std::get<0>(check);
    int valR = std::get<1>(check);
    int valG = std::get<2>(check);
    int valB = std::get<3>(check);

    if ((!quad && sw > 8 && sh > 8) || (sw > 32 && sh > 32)) {
        uint16_t sw_l = sw / 2;
        uint16_t sw_r = (sw + 1) / 2;
        uint16_t sh_t = sh / 2;
        uint16_t sh_b = (sh + 1) / 2;
        subdivideRGB(sx, sy, sw_l, sh_t, frameRGB, resizedAmogi);
        subdivideRGB(sx + sw_r, sy, sw_l, sh_t, frameRGB, resizedAmogi);
        subdivideRGB(sx, sy + sh_b, sw_l, sh_t, frameRGB, resizedAmogi);
        subdivideRGB(sx + sw_r, sy + sh_b, sw_l, sh_t, frameRGB, resizedAmogi);
    } else {
        frameRGB.overlay(resizedAmogi[std::make_pair(sw, sh)].colorMaskNew(valR / 255.f, valG / 255.f, valB / 255.f),
                         sx, sy);
    }
}

std::tuple<bool, int, int, int> Image::subdivideCheckRGB(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh) const {
    bool quad = true;
    uint8_t colR = data.at((sx + sy * w) * channels);
    uint8_t colG = data.at((sx + sy * w) * channels + 1);
    uint8_t colB = data.at((sx + sy * w) * channels + 2);
    int sumR = 0;
    int sumG = 0;
    int sumB = 0;

    for (uint16_t y = sy; y < sh + sy; y++) {
        for (uint16_t x = sx; x < sw + sx; x++) {
            uint8_t pixR = data.at((x + y * w) * channels);
            sumR += pixR;
            if (colR != pixR)
                quad = false;
            uint8_t pixG = data.at((x + y * w) * channels + 1);
            sumG += pixG;
            if (colG != pixG)
                quad = false;
            uint8_t pixB = data.at((x + y * w) * channels + 2);
            sumB += pixB;
            if (colB != pixB)
                quad = false;
        }
    }

    return std::make_tuple(quad, (int)sumR / (sh * sw), (int)sumG / (sh * sw), (int)sumB / (sh * sw));
}

void Image::subdivideValues(int sx, int sy, int sw, int sh, std::map<std::pair<int, int>, Image> &image_map) const {
    if (sw > 4 && sh > 4) {
        subdivideValues(sx, sy, sw / 2, sh / 2, image_map);
    }

    if (image_map.count(std::make_pair(sw, sh)) == 0) {
        image_map[std::make_pair(sw, sh)] = resizeFastNew(sw, sh);
        // std::cout<<sw<<" "<<sh<<" : "<<image_map.count(std::make_pair(sw, sh))<<" "<<image_map.size()<<"\n";
    }
}

std::map<std::pair<int, int>, Image> Image::preloadResized(int sw, int sh) const {
    std::map<std::pair<int, int>, Image> image_map;

    subdivideValues(0, 0, sw, sh, image_map);

    return image_map;
}
