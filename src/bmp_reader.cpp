#include "bmp_reader.h"

#include <cstdlib>
#include <ios>
#include <iostream>
#include <limits>
#include <string>
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
    } else if (h_size == 40 || h_size == 108 || h_size == 124) {
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
    imp_fields.bit_count = core_header.bit_count;
}

void BMPReader::ReadNewInfoHeader() {
    util::BitmapInfoHeader info_header;
    *is_ >> info_header;

    imp_fields.width = info_header.width;
    imp_fields.height = std::abs(info_header.height);
    imp_fields.bottom_up = info_header.height > 0;
    imp_fields.bit_count = info_header.bit_count;
    imp_fields.compression = static_cast<Compression>(info_header.compression);
    // If size_image is 0, it will be calculated later
    imp_fields.size_image = info_header.size_image;
    if (info_header.clr_used > 0) {
        imp_fields.palette_used = true;
        imp_fields.palette_important = info_header.clr_important;
    }
}

std::vector<std::vector<RGBColor>> BMPReader::Read24bitData() {
    std::vector<std::vector<RGBColor>> bit_data;
    for (std::size_t scan_num = 0; scan_num < imp_fields.height; ++scan_num) {
        std::vector<RGBColor> scan;
        for (std::size_t cell_num = 0; cell_num < imp_fields.width; ++cell_num) {
            RGBColor color;
            is_->read(reinterpret_cast<char*>(&color), sizeof(color));
            scan.push_back(color);
        }
        if (imp_fields.padding_bytes > 0) {
            is_->ignore(imp_fields.padding_bytes);
        }
        bit_data.push_back(scan);
    }
    return bit_data;
}
}  // namespace bmp
