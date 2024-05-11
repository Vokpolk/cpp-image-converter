#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    // поля заголовка Bitmap File Header
    char header[2];
    unsigned int size_header_and_info;
    int reserved;
    unsigned int indent;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader{
    // поля заголовка Bitmap Info Header
    unsigned int size;
    int width;
    int height;
    unsigned short flat;
    unsigned short bits_per_pixel;
    unsigned int compression;
    unsigned int byte_count;
    unsigned int horiz_resolution;
    unsigned int vert_resolution;
    int num_of_used_colors;
    int num_of_significant_colors;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    int colors_count = 3;
    int leveling = 4;
    int round_up = 3;

    return leveling * ((w * colors_count + round_up) / leveling);
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    
    BitmapFileHeader file_;
    file_.header[0] = 'B';
    file_.header[1] = 'M';
    file_.size_header_and_info = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + GetBMPStride(image.GetWidth()) * image.GetHeight();
    file_.reserved = 0;
    file_.indent = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
    BitmapInfoHeader info_;
    info_.size = sizeof(BitmapInfoHeader);
    info_.width = image.GetWidth();
    info_.height = image.GetHeight();
    info_.flat = 1;
    info_.bits_per_pixel = 24;
    info_.compression = 0;
    info_.byte_count = GetBMPStride(image.GetWidth()) * image.GetHeight();
    info_.horiz_resolution = 11811;
    info_.vert_resolution = 11811;
    info_.num_of_used_colors = 0;
    info_.num_of_significant_colors = 0x1000000;

    out.write(reinterpret_cast<const char*>(&file_), sizeof(file_));
    out.write(reinterpret_cast<const char*>(&info_), sizeof(info_));

    const int w = image.GetWidth();
    const int h = image.GetHeight();

    int bmp_stride = GetBMPStride(image.GetWidth());
    std::vector<char> buff(bmp_stride);

    for (int y = h - 1; y >= 0; y--) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; x++) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), bmp_stride);
    }


    return out.good();
}

Image LoadBMP(const Path& file) {
    // открываем поток с флагом ios::binary
    // поскольку будем читать данные в двоичном формате
    ifstream ifs(file, ios::binary);

    BitmapFileHeader file_;
    BitmapInfoHeader info_;
    ifs.read(reinterpret_cast<char*>(&file_), sizeof(file_));
    ifs.read(reinterpret_cast<char*>(&info_), sizeof(info_));

    if (file_.header[0] != 'B' && file_.header[1] != 'M') {
        return {};
    }

    const int w = info_.width;
    const int h = info_.height;
    int bmp_stride = GetBMPStride(w);

    Image result(w, h, Color::Black());

    std::vector<char> buff(bmp_stride);

    for (int y = h - 1; y >= 0; y--) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), bmp_stride);

        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib