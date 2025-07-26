#pragma once

#include <algorithm>
#include <bit>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <vector>

#include "util/color.h"
#include "util/field_types.h"
#include "util/ms_constants.h"

namespace bmp {
/// @brief Reads BMP file into 2D bitset
class BMPReader {
private:
    /// @brief Holds header fields that are used by Reader
    /// "New version" fields are pre-initialized (except for size_image, that must be calculated)
    struct ImportantFields {
        // Pixel data offset (bytes)
        DWord offset;
        // Width and height
        DWord width;
        DWord height;
        // Scans order (true is bottom-up, false is top-down)
        bool bottom_up = true;
        // Bits per pixel
        Word bit_count;
        util::Compression compression = util::Compression::RGB;
        // Data size (bytes). CANNOT be zero
        DWord size_image = 0;
        // Palette
        bool palette_used = false;
        DWord palette_important = 0;
        // Scans must be aligned to 32 bits
        Word padding_bytes = 0;
    };

    std::istream* is_;
    ImportantFields imp_fields;
    std::vector<std::vector<bool>> pixel_data_;

    DWord file_size_;

    void ReadFileHeader();
    void ReadInfoHeader();

    void ReadCoreInfoHeader();
    void ReadNewInfoHeader();

    [[nodiscard]] std::vector<std::vector<util::RGBColor>> Read24bitData();

public:
    /// @param is -- @c std::istream to read from
    /// @param file_size -- input file size, used only to check header. Set to 0 to disable checks
    BMPReader(std::istream& is, DWord file_size = 0) : is_(&is), file_size_(file_size) {
        if (std::endian::native != std::endian::little) {
            // TODO: Big-endian
            throw std::logic_error("Only little-endian platforms are supported now");
        }
    }

    void ReadHeaders() {
        ReadFileHeader();
        ReadInfoHeader();

        if (imp_fields.size_image == 0) {
            imp_fields.size_image = imp_fields.width * imp_fields.height * imp_fields.bit_count / 8;
        }

        if (imp_fields.bit_count != 32) {
            imp_fields.padding_bytes = imp_fields.width * imp_fields.bit_count / 8 % 4;
            if (imp_fields.padding_bytes > 0) {
                imp_fields.padding_bytes = 4 - imp_fields.padding_bytes;
            }
        }
    }

    void ReadData() {
        is_->seekg(imp_fields.offset);

        std::vector<std::vector<util::RGBColor>> bit_data;
        if (imp_fields.bit_count == 24) {
            bit_data = Read24bitData();
        } else {
            // TODO: 32-bit data
            throw std::logic_error("32-bit BMP isn't supported yet");
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

    std::vector<std::vector<bool>> const& GetPixelData() const {
        return pixel_data_;
    }
};
}  // namespace bmp
