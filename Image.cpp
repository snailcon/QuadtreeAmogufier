#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define BYTE_BOUND(value) value < 0 ? 0 : (value > 255 ? 255 : value)

#include "Image.h"

#include "lib/stb_image.h"
#include "lib/stb_image_write.h"

Image::Image() : w(100), h(100), channels(3) {
    size = w*h*channels;
    data = std::vector<uint8_t>(size);
}

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
    data = std::vector<uint8_t>(size);
}

Image::Image(const Image& img) : w(img.w), h(img.h), channels(img.channels), data(img.data) {
    size = w*h*channels;
}

bool Image::read(const char* filename) {
    uint8_t* temp = stbi_load(filename, &w, &h, &channels, 0);
    size = w*h*channels;
    data.insert(data.end(), &temp[0], &temp[size]);
    stbi_image_free(temp);
    return true;
}

bool Image::write(const char* filename) const {
    int success;
    success = stbi_write_png(filename, w, h, channels, data.data(), w*channels);
    return success != 0;
}

Image& Image::colorMask(float r, float g, float b) {
    for (int i = 0; i < size; i+=channels) {
        data.at(i)   *= r;
        data.at(i+1) *= g;
        data.at(i+2) *= b;
    }
    return *this;
}

Image Image::colorMaskNew(float r, float g, float b) {
    Image new_version = *this;
    for (int i = 0; i < size; i+=channels) {
        new_version.data.at(i)   *= r;
        new_version.data.at(i+1) *= g;
        new_version.data.at(i+2) *= b;
    }
    return new_version;
}

Image& Image::overlay(const Image& source, int x, int y) {

    for (int sy = 0; sy < source.h; sy++) {
        if (sy + y < 0) continue; else if (sy + y >= h) break;
        for (int sx = 0; sx < source.w; sx++) {
            if (sx + x < 0) continue; else if (sx + x >= w) break;

            float srcAlpha = source.channels < 4 ? 1 : source.data.at(((sx + sy * source.w) * source.channels) + 3) / 255.f;
            float dstAlpha = channels < 4 ? 1 : data.at(((sx + x + (sy + y) * w) * channels) + 3) / 255.f;

            if (srcAlpha > .99 && dstAlpha > .99) {
                for (int channel = 0; channel < channels; channel++) {
                    data.at(((sx + x + (sy + y) * w) * channels) + channel) = source.data.at(((sx + sy * source.w) * source.channels) + channel);
                }
            } else {
                float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);
                if (outAlpha < .01) {
                    for (int channel = 0; channel < channels; channel++) {
                        data.at(((sx + x + (sy + y) * w) * channels) + channel) = 0;
                    }
                } else {
                    for (int channel = 0; channel < channels; channel++) {
                        data.at(((sx + x + (sy + y) * w) * channels) + channel) = (uint8_t)BYTE_BOUND((source.data.at(((sx + sy * source.w) * source.channels) + channel)/255.f * srcAlpha + data.at(((sx + x + (sy + y) * w) * channels) + channel)/255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255.f);
                    }
                    if (channels > 3) data.at(((sx + x + (sy + y) * w) * channels) + 3) = (uint8_t)BYTE_BOUND(outAlpha * 255.f);
                }
            }
        }
    }

    return *this;
}

Image& Image::resizeFast(uint16_t rw, uint16_t rh) {
    std::vector<uint8_t> resizedImage(rw * rh * channels);

    double x_ratio = w/(double)rw;
    double y_ratio = h/(double)rh;
    double rx, ry ;
    for (int y = 0; y < rh; y++) {
        for (int x = 0; x < rw; x++) {
            rx = floor(x * x_ratio);
            ry = floor(y * y_ratio);
            for (int channel = 0; channel < channels; channel++) {
                resizedImage.at((((y*rw)+x) * channels) + channel) = data.at((((ry*w)+rx) * channels) + channel);
            }
        }                
    }          

    w = rw;
    h = rh;
    size = w * h * channels;

    data = resizedImage;

    return *this;
}

Image Image::resizeFastNew(uint16_t rw, uint16_t rh) {
    Image new_version = *this;
    std::vector<uint8_t> resizedImage(rw * rh * channels);

    double x_ratio = w/(double)rw;
    double y_ratio = h/(double)rh;
    double rx, ry ;
    for (int y = 0; y < rh; y++) {
        for (int x = 0; x < rw; x++) {
            rx = floor(x * x_ratio);
            ry = floor(y * y_ratio);
            for (int channel = 0; channel < channels; channel++) {
                resizedImage.at((((y*rw)+x) * channels) + channel) = data.at((((ry*w)+rx) * channels) + channel);
            }
        }                
    }           

    new_version.w = rw;
    new_version.h = rh;
    new_version.size = rw * rh * channels;

    new_version.data = resizedImage;

    return new_version;
}

Image Image::cropNew(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch) {
    Image new_version = *this;

    std::vector<uint8_t> croppedImage(cw * ch * channels);

    for (uint16_t y = 0; y < ch; y++) {
        if (y + cy >= h) break;
        for (uint16_t x = 0; x < cw; x++) {
            if (x + cx >= w) break;
            for (int channel = 0; channel < channels; channel++) {
                croppedImage.at(((x + y * cw) * channels) + channel) = data.at(((x + cx + (y + cy) * w) * channels) + channel);
            }
        }
    }

    new_version.w = cw;
    new_version.h = ch;
    new_version.size = cw*ch*channels;

    new_version.data = croppedImage;

    return new_version;
}

Image& Image::rect(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t colors[] = {r, g, b, 255}; 
    for (uint16_t y = cy; y < ch + cy; y++) {
        if (y >= h) break;
        for (uint16_t x = cx; x < cw + cx; x++) {
            if (x >= w) break;
            for (int channel = 0; channel < channels; channel++) {
                data.at(((x + y * w) * channels) + channel) = colors[channel];
            }
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
            for (int channel = 0; channel < channels; channel++) {
                data.at(((x + y * w) * channels) + channel) = colors[channel];
            }
        }
    }

    for (uint16_t x = cx; x < cw + cx; x+=cw-1) {
        if (x >= w) break;
        for (uint16_t y = cy; y < ch + cy; y++) {
            if (y >= h) break;
            for (int channel = 0; channel < channels; channel++) {
                data.at(((x + y * w) * channels) + channel) = colors[channel];
            }
        }
    }

    return *this;
}

Image& Image::rect(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t colors [4] = {r, g, b, 255}; 
    for (uint16_t y = 0; y < h; y++) {
        for (uint16_t x = 0; x < w; x++) {
            for (int channel = 0; channel < channels; channel++) {
                data.at(((x + y * w) * channels) + channel) = colors[channel];
            }
        }
    }

    return *this;
}

Image& Image::rectOutline(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t colors [4] = {r, g, b, 255}; 
    for (uint16_t y = 0; y < h; y+=h-1) {
        for (uint16_t x = 0; x < w; x++) {
            for (int channel = 0; channel < channels; channel++) {
                data.at(((x + y * w) * channels) + channel) = colors[channel];
            }
        }
    }

    for (uint16_t x = 0; x < w; x+=w-1) {
        for (uint16_t y = 0; y < h; y++) {
            for (int channel = 0; channel < channels; channel++) {
                data.at(((x + y * w) * channels) + channel) = colors[channel];
            }
        }
    }

    return *this;
}

Image Image::quadifyFrameBW(std::map<std::pair<int, int>, Image>& resizedAmogi) {
    Image frame(w, h, 3);

    subdivideBW(0, 0, w, h, frame, resizedAmogi);

    return frame;
}

// sw: subdivided x | sy subdivided y
// sw: subdivided width | sh subdivided height
void Image::subdivideBW(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, Image& frame, std::map<std::pair<int, int>, Image>& resizedAmogi) {

    int val = subdivideCheckBW(sx, sy, sw, sh);

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
        subdivideBW(sx, sy, sw_l, sh_t, frame, resizedAmogi);
        subdivideBW(sx + sw_r, sy, sw_l, sh_t, frame, resizedAmogi);
        subdivideBW(sx, sy + sh_b, sw_l, sh_t, frame, resizedAmogi);
        subdivideBW(sx + sw_r, sy + sh_b, sw_l, sh_t, frame, resizedAmogi);
    } else {
        if (val <= 20) return;
        frame.overlay(resizedAmogi[std::make_pair(sw, sh)].colorMaskNew(val/255.f, val/255.f, val/255.f), sx, sy);
    }
}

int Image::subdivideCheckBW(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh) {
    int sum = 0;

    for (uint16_t y = sy; y < sh + sy; y++) {
        for (uint16_t x = sx; x < sw + sx; x++) {
            sum += data.at((x + y * w) * channels);
        }
    }

    return (int)sum/(sh*sw);
}

Image Image::quadifyFrameRGB(std::map<std::pair<int, int>, Image>& resizedAmogi) {
    Image frameRGB(w, h, 3);

    subdivideRGB(0, 0, w, h, frameRGB, resizedAmogi);

    return frameRGB;
}

void Image::subdivideRGB(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, Image& frameRGB, std::map<std::pair<int, int>, Image>& resizedAmogi) {

    std::tuple<bool, int, int, int> check = subdivideCheckRGB(sx, sy, sw, sh);
    bool quad = std::get<0>(check);
    int valR = std::get<1>(check);
    int valG = std::get<2>(check);
    int valB = std::get<3>(check);

    if ((!quad && sw > 8 && sh > 8) || (sw > 32 && sh > 32)) {
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
        subdivideRGB(sx, sy, sw_l, sh_t, frameRGB, resizedAmogi);
        subdivideRGB(sx + sw_r, sy, sw_l, sh_t, frameRGB, resizedAmogi);
        subdivideRGB(sx, sy + sh_b, sw_l, sh_t, frameRGB, resizedAmogi);
        subdivideRGB(sx + sw_r, sy + sh_b, sw_l, sh_t, frameRGB, resizedAmogi);
    } else {
        frameRGB.overlay(resizedAmogi[std::make_pair(sw, sh)].colorMaskNew(valR/255.f, valG/255.f, valB/255.f), sx, sy);
    }
}

std::tuple<bool, int, int, int> Image::subdivideCheckRGB(uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh) {
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
            if (colR != pixR) quad = false;
            uint8_t pixG = data.at((x + y * w) * channels + 1);
            sumG += pixG;
            if (colG != pixG) quad = false;
            uint8_t pixB = data.at((x + y * w) * channels + 2);
            sumB += pixB;
            if (colB != pixB) quad = false;
        }
    }

    return std::make_tuple(quad, (int)sumR/(sh*sw), (int)sumG/(sh*sw), (int)sumB/(sh*sw));
}


void Image::subdivideValues(int sx, int sy, int sw, int sh, std::map<std::pair<int, int>, Image>& image_map) {
    if (sw > 4 && sh > 4) {
        int sw_l, sw_r, sh_t, sh_b;
        if (sw % 2 == 0) {
            sw_l = sw/2;
        } else {
            sw_l = floor(sw/2);
        }
        if (sh % 2 == 0) { 
            sh_t = sh/2;
        } else {
            sh_t = floor(sh/2);
        }
        subdivideValues(sx, sy, sw_l, sh_t, image_map);
    }

    if (image_map.count(std::make_pair(sw, sh)) == 0) {
        image_map[std::make_pair(sw, sh)] = resizeFastNew(sw, sh);
        // std::cout<<sw<<" "<<sh<<" : "<<image_map.count(std::make_pair(sw, sh))<<" "<<image_map.size()<<"\n";
    }
}

std::map<std::pair<int, int>, Image> Image::preloadResized(int sw, int sh) {
    std::map<std::pair<int, int>, Image> image_map;

    subdivideValues(0, 0, sw, sh, image_map);

    return image_map;
}
