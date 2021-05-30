#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define BYTE_BOUND(value) value < 0 ? 0 : (value > 255 ? 255 : value)

#include "Image.h"

#include "lib/stb_image.h"
#include "lib/stb_image_write.h"

Image::Image(const char* filename) {
    if(read(filename)) {
        // std::cout<<"Read "<<filename<<" Width: "<<w<<" Height: "<<h<<" Channels: "<<channels<<std::endl;
        size = w*h*channels;
    } else {
        std::cout<<"Failed to read "<<filename<<std::endl;
    }
}

Image::Image(int w, int h, int channels) : w(w), h(h), channels(channels) {
    size = w*h*channels;
    data = new uint8_t[size];
}

Image::Image(const Image& img) : Image(img.w, img.h, img.channels) {
    memcpy(data, img.data, size);
}

Image::~Image() {
    stbi_image_free(data);
    // delete [] data;
}

bool Image::read(const char* filename) {
    data = stbi_load(filename, &w, &h, &channels, 0);
    return data != NULL;
}

bool Image::write(const char* filename) {
    int success;
    success = stbi_write_png(filename, w, h, channels, data, w*channels);
    return success != 0;
}

Image& Image::colorMask(float r, float g, float b) {
    for (int i = 0; i < size; i+=channels) {
        data[i]   *= r;
        data[i+1] *= g;
        data[i+2] *= b;
    }
    return *this;
}

Image Image::colorMaskNew(float r, float g, float b) {
    Image new_version = *this;
    for (int i = 0; i < size; i+=channels) {
        new_version.data[i]   *= r;
        new_version.data[i+1] *= g;
        new_version.data[i+2] *= b;
    }
    return new_version;
}

Image& Image::overlay(const Image& source, int x, int y) {

    uint8_t* srcPx;
    uint8_t* dstPx;

    for (int sy = 0; sy < source.h; sy++) {
        if (sy + y < 0) continue; else if (sy + y >= h) break;
        for (int sx = 0; sx < source.w; sx++) {
            if (sx + x < 0) continue; else if (sx + x >= w) break;
            srcPx = &source.data[(sx + sy * source.w) * source.channels];
            dstPx = &data[(sx + x + (sy + y) * w) * channels];

            float srcAlpha = source.channels < 4 ? 1 : srcPx[3] / 255.f;
            float dstAlpha = channels < 4 ? 1 : dstPx[3] / 255.f;

            if (srcAlpha > .99 && dstAlpha > .99) {
                memcpy(dstPx, srcPx, channels);
            } else {
                float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);
                if (outAlpha < .01) {
                    memset(dstPx, 0, channels);
                } else {
                    for (int channel = 0; channel < channels; channel++) {
                        dstPx[channel] = (uint8_t)BYTE_BOUND((srcPx[channel]/255.f * srcAlpha + dstPx[channel]/255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255.f);
                    }
                    if (channels > 3) dstPx[3] = (uint8_t)BYTE_BOUND(outAlpha * 255.f);
                }
            }
        }
    }

    return *this;
}

Image& Image::resizeFast(uint16_t rw, uint16_t rh) {
    uint8_t* resizedImage = new uint8_t[(int)ceil(rw * rh * channels)];

    double x_ratio = w/(double)rw;
    double y_ratio = h/(double)rh;
    double rx, ry ;
    for (int y = 0; y < rh; y++) {
        for (int x = 0; x < rw; x++) {
            rx = floor(x * x_ratio);
            ry = floor(y * y_ratio);
            memcpy(&resizedImage[((y*rw)+x) * channels], &data[((int)((ry*w)+rx)) * channels], channels);
        }                
    }          

    w = rw;
    h = rh;
    size = w * h * channels;

    delete [] data;
    data = resizedImage;
    resizedImage = nullptr;

    return *this;
}

Image Image::resizeFastNew(uint16_t rw, uint16_t rh) {
    Image new_version = *this;
    uint8_t* resizedImage = new uint8_t[(int)ceil(rw * rh * channels)];

    double x_ratio = w/(double)rw;
    double y_ratio = h/(double)rh;
    double rx, ry ;
    for (int y = 0; y < rh; y++) {
        for (int x = 0; x < rw; x++) {
            rx = floor(x * x_ratio);
            ry = floor(y * y_ratio);
            memcpy(&resizedImage[((y*rw)+x) * channels], &data[((int)((ry*w)+rx)) * channels], channels);
        }                
    }          

    new_version.w = rw;
    new_version.h = rh;
    new_version.size = w * h * channels;

    delete [] new_version.data;
    new_version.data = resizedImage;
    resizedImage = nullptr;

    return new_version;
}

Image Image::cropNew(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch) {
    Image new_version = *this;

    uint8_t* croppedImage = new uint8_t[cw * ch * channels];

    memset(croppedImage, 0, cw * ch * channels);

    for (uint16_t y = 0; y < ch; y++) {
        if (y + cy >= h) break;
        for (uint16_t x = 0; x < cw; x++) {
            if (x + cx >= w) break;
            memcpy(&croppedImage[(x + y * cw) * channels], &data[(x + cx + (y + cy) * w) * channels], channels);
        }
    }

    new_version.w = cw;
    new_version.h = ch;
    new_version.size = cw*ch*channels;

    delete [] new_version.data;
    new_version.data = croppedImage;
    croppedImage = nullptr;

    return new_version;
}

Image& Image::rect(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t colors[] = {r, g, b, 255}; 
    for (uint16_t y = cy; y < ch + cy; y++) {
        if (y >= h) break;
        for (uint16_t x = cx; x < cw + cx; x++) {
            if (x >= w) break;
            memcpy(&data[(x + y * w) * channels], &colors, channels);
        }
    }

    return *this;
}

Image& Image::rectOutline(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch, uint8_t r, uint8_t g, uint8_t b) {

    if (cw <= 1 || ch <= 1) {
        return rect(cx, cy, cw, ch, r, g, b);
    }

    uint8_t colors [4] = {r, g, b, 255}; 
    for (uint16_t y = cy; y < ch + cy; y+=ch-1) {
        if (y >= h) break;
        for (uint16_t x = cx; x < cw + cx; x++) {
            if (x >= w) break;
            memcpy(&data[(x + y * w) * channels], &colors, channels);
        }
    }

    for (uint16_t x = cx; x < cw + cx; x+=cw-1) {
        if (x >= w) break;
        for (uint16_t y = cy; y < ch + cy; y++) {
            if (y >= h) break;
            memcpy(&data[(x + y * w) * channels], &colors, channels);
        }
    }

    return *this;
}

Image& Image::rect(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t colors [4] = {r, g, b, 255}; 
    for (uint16_t y = 0; y < h; y++) {
        for (uint16_t x = 0; x < w; x++) {
            memcpy(&data[(x + y * w) * channels], &colors, channels);
        }
    }

    return *this;
}

Image& Image::rectOutline(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t colors [4] = {r, g, b, 255}; 
    for (uint16_t y = 0; y < h; y+=h-1) {
        for (uint16_t x = 0; x < w; x++) {
            memcpy(&data[(x + y * w) * channels], &colors, channels);
        }
    }

    for (uint16_t x = 0; x < w; x+=w-1) {
        for (uint16_t y = 0; y < h; y++) {
            memcpy(&data[(x + y * w) * channels], &colors, channels);
        }
    }

    return *this;
}

Image Image::quadifyFrame(int i) {
    Image frame(w, h, 3); // empty black frame

    int index = floor((i % 12)/2);
    std::string amogus_frame("res/" + std::to_string(index) + "_big.png");
    std::string amogus_eye_frame("res/" + std::to_string(index) + "_eye_big.png");
    Image amogus(amogus_frame.c_str());
    Image amogus_eye(amogus_eye_frame.c_str());

    subdivide(0, 0, w, h, i, frame, amogus, amogus_eye);

    return frame;
}

// sw: subdivided x | sy subdivided y
// sw: subdivided width | sh subdivided height
void Image::subdivide(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, int i, Image& frame, Image& amogus, Image& amogus_eye) {

    int val = subdivideCheck(sx, sy, sw, sh);

    if (val > 0 && val < 255 && sw > 16 && sh > 16) {
        uint16_t sw_l, sw_r, sh_t, sh_b;
        if (sw % 2 == 0) {
            sw_l = sw/2;
            sw_r = sw/2; 
        } else {
            sw_l = floor(sw/2);
            sw_r = ceil(sw/2) + 1;
        }
        if (sh % 2 == 0) { 
            sh_t = sh/2;
            sh_b = sh/2;
        } else {
            sh_t = floor(sh/2);
            sh_b = ceil(sh/2) + 1;
        }
        subdivide(sx, sy, sw_l, sh_t, i, frame, amogus, amogus_eye);
        subdivide(sx + sw_r, sy, sw_l, sh_t, i, frame, amogus, amogus_eye);
        subdivide(sx, sy + sh_b, sw_l, sh_t, i, frame, amogus, amogus_eye);
        subdivide(sx + sw_r, sy + sh_b, sw_l, sh_t, i, frame, amogus, amogus_eye);
    } else {
        if (val <= 20) return;
        Image local_amogus_eye = amogus_eye.resizeFastNew(sw, sh);
        Image local_amogus = amogus.colorMaskNew(val/255.f, val/255.f, val/255.f);
        local_amogus.resizeFast(sw, sh);
        frame.overlay(local_amogus, sx, sy);
        frame.overlay(local_amogus_eye, sx, sy);
    }
}

int Image::subdivideCheck(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh) { // returns average color in quarter
    int sum = 0;

    for (uint16_t y = sy; y < sh + sy; y++) {
        for (uint16_t x = sx; x < sw + sx; x++) {
            sum += data[(x + y * w) * channels];
        }
    }

    return (int)sum/(sh*sw);
}
