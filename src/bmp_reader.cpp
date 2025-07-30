#include "bmp_reader.h"

#include <algorithm>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <istream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "util/bitmap_file_header.h"
#include "util/bitmap_info_header.h"
#include "util/color.h"
#include "util/field_types.h"
#include "util/invalid_bmp_error.h"
#include "util/io_error.h"
#include "util/ms_constants.h"

namespace bmp {

using namespace util;

void BMPReader::ReadFileHeader() {
    util::BitmapFileHeader file_header;
    *is_ >> file_header;

    if (file_size_ > 0) {
        InvalidBMPError::Assert(file_header.file_size == file_size_,
                                "invalid file size: requested " + std::to_string(file_size_) +
                                        ", header says " + std::to_string(file_header.file_size));
    }
    imp_fields.offset = file_header.offset;
}

void BMPReader::ReadInfoHeader() {
    DWord h_size;
    if (!is_->read(reinterpret_cast<char*>(&h_size), sizeof(h_size))) {
        throw IOError("cannot read info header size");
    }

    if (h_size == 12) {
        ReadCoreInfoHeader();
        // 54 and 56-bit headers are not documented by Microsoft, but are used sometimes
    } else if (h_size == 40 || h_size == 54 || h_size == 56 || h_size == 108 || h_size == 124) {
        ReadNewInfoHeader();
    } else {
        throw InvalidBMPError("invalid info header size: " + std::to_string(h_size));
    }
}

void BMPReader::ReadCoreInfoHeader() {
    util::BitmapCoreHeader core_header;
    *is_ >> core_header;

    imp_fields.width = core_header.width;
    imp_fields.height = core_header.height;
    imp_fields.byte_count = core_header.bit_count / 8;
}

void BMPReader::ReadNewInfoHeader() {
    util::BitmapInfoHeader info_header;
    *is_ >> info_header;

    imp_fields.width = info_header.width;
    imp_fields.height = std::abs(info_header.height);
    imp_fields.bottom_up = info_header.height > 0;
    imp_fields.byte_count = info_header.bit_count / 8;
    imp_fields.compression = static_cast<Compression>(info_header.compression);
    // If size_image is 0, it will be calculated later
    imp_fields.size_image = info_header.size_image;
    if (info_header.clr_used > 0) {
        imp_fields.palette_used = true;
        imp_fields.palette_important = info_header.clr_important;
    }

    // Check masks
    if (imp_fields.byte_count == 4 && imp_fields.compression == Compression::BITFIELDS ||
        imp_fields.compression == Compression::ALPHABITFIELDS) {
        DWord r_mask, g_mask, b_mask;
        if (!is_->read(reinterpret_cast<char*>(&r_mask), sizeof(r_mask)) ||
            !is_->read(reinterpret_cast<char*>(&g_mask), sizeof(g_mask)) ||
            !is_->read(reinterpret_cast<char*>(&b_mask), sizeof(b_mask))) {
            throw IOError("cannot read bit mask");
        }
        if (r_mask != kStandRMask || g_mask != kStandGMask || b_mask != kStandBMask) {
            throw InvalidBMPError("non-standard bit masks are not supported yet");
        }
    }
}

void BMPReader::ReadData() {
    is_->seekg(imp_fields.offset);

    std::vector<std::vector<RGBColor>> bit_data;
    for (std::size_t scan_num = 0; scan_num < imp_fields.height; ++scan_num) {
        std::vector<RGBColor> scan;
        for (std::size_t cell_num = 0; cell_num < imp_fields.width; ++cell_num) {
            RGBColor color;
            if (imp_fields.byte_count == 3) {
                color = Read24bitPixel();
            } else {
                color = Read32bitPixel();
            }
            scan.push_back(color);
        }
        if (imp_fields.padding_bytes > 0) {
            is_->ignore(imp_fields.padding_bytes);
        }
        bit_data.push_back(scan);
    }

    if (imp_fields.bottom_up) {
        std::reverse(bit_data.begin(), bit_data.end());
    }

    for (auto const& scan : bit_data) {
        std::vector<bool> black_and_white_scan;
        std::transform(scan.begin(), scan.end(), std::back_inserter(black_and_white_scan),
                       [](util::RGBColor color) {
                           return (color.red + color.green + color.blue) <= 122 * 3;
                       });
        pixel_data_.push_back(std::move(black_and_white_scan));
    }
}

RGBColor BMPReader::Read24bitPixel() {
    RGBColor color;
    if (!is_->read(reinterpret_cast<char*>(&color), sizeof(color))) {
        throw IOError("cannot read pixel data");
    }
    return color;
}

RGBColor BMPReader::Read32bitPixel() {
    RGBColor color;
    DWord raw_color;
    if (!is_->read(reinterpret_cast<char*>(&raw_color), sizeof(raw_color))) {
        throw IOError("cannot read pixel data");
    }
    color.red = raw_color & kStandRMask >> 16;
    color.green = raw_color & kStandGMask >> 8;
    color.blue = raw_color & kStandBMask;
    return color;
}

void BMPReader::DrawPixel(DWord x, DWord y) {
    // Draw on pixel data (note: it's top-down)
    auto rev_y = imp_fields.height - y - 1;
    pixel_data_[rev_y][x] = true;

    // Draw on BMP contents
    auto const prev_scans = y == 0 ? 0 : y - 1;
    auto const full_scan = imp_fields.width * imp_fields.byte_count + imp_fields.padding_bytes;
    auto const pos = imp_fields.offset + prev_scans * full_scan + x * imp_fields.byte_count;
    bmp_contents_.seekp(pos);

    constexpr static util::RGBColor Black24bitPx{0, 0, 0};
    constexpr static DWord Black32bitPx = 0;
    if (imp_fields.byte_count == 3) {
        bmp_contents_.write(reinterpret_cast<char const*>(&Black24bitPx), sizeof(Black24bitPx));
    } else {
        bmp_contents_.write(reinterpret_cast<char const*>(&Black32bitPx), sizeof(Black32bitPx));
    }
}

void BMPReader::DrawLine(DWord x1, DWord y1, DWord x2, DWord y2) {
    using Point = std::pair<DWord, DWord>;
    std::vector<Point> line;

    DWord x_diff = std::abs(static_cast<long>(x2) - x1);
    DWord y_diff = std::abs(static_cast<long>(y2) - y1);
    if (x_diff >= y_diff) {
        // k <= 1
        if (x1 > x2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        for (auto x = x1; x <= x2; ++x) {
            auto y_shift = x_diff == 0 ? 0 : (x - x1) * y_diff / x_diff;
            if (y1 > y2) {
                y_shift *= -1;
            }
            auto y = y1 + y_shift;
            line.emplace_back(x, y);
        }
    } else {
        std::cout << "k > 1\n";
        if (y1 > y2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        for (auto y = y1; y <= y2; ++y) {
            auto x_shift = y_diff == 0 ? 0 : (y - y1) * x_diff / y_diff;
            if (x1 > x2) {
                x_shift *= -1;
            }
            auto x = x1 + x_shift;
            line.emplace_back(x, y);
        }
    }

    // Actually draw
    for (auto const p : line) {
        DrawPixel(p.first, p.second);
    }
}
}  // namespace bmp
