#pragma once

#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <sstream>
#include <unistd.h>
#include <utility>
#include <vector>

#include "util/color.h"
#include "util/field_types.h"
#include "util/ms_constants.h"

namespace bmp {
/// @brief Reads BMP file into 2D bitset
class BMPReader {
public:
    /// @brief Holds header fields that are used by Reader
    /// "New version" fields are pre-initialized (except for size_image, that must be calculated)
    /// @note This class is made public for testing purposes
    struct ImportantFields {
        // Pixel data offset (bytes)
        DWord offset;
        // Width and height
        DWord width;
        DWord height;
        // Scans order (true is bottom-up, false is top-down)
        bool bottom_up = true;
        // Bytes per pixel
        Word byte_count;
        util::Compression compression = util::Compression::RGB;
        // Data size (bytes). CANNOT be zero
        DWord size_image = 0;
        // Palette
        bool palette_used = false;
        DWord palette_important = 0;
        // Scans must be aligned to 32 bits
        Word padding_bytes = 0;
    };

private:
    // Standard masks
    constexpr static DWord kStandRMask = 0x00FF0000;
    constexpr static DWord kStandGMask = 0x0000FF00;
    constexpr static DWord kStandBMask = 0x000000FF;

    std::istream* is_;
    ImportantFields imp_fields;
    std::vector<std::vector<bool>> pixel_data_;
    // Holds the whole BMP contents and is being edited on Draw*
    std::stringstream bmp_contents_;

    DWord file_size_;

    void ReadFileHeader();
    void ReadInfoHeader();

    void ReadCoreInfoHeader();
    void ReadNewInfoHeader();

    [[nodiscard]] util::RGBColor Read24bitPixel();
    [[nodiscard]] util::RGBColor Read32bitPixel();

    /// @note Bottom-up coordinates are used (i. e. bottom-left corner is 0)
    void DrawPixel(DWord x, DWord y);

    void DrawLine(DWord x1, DWord y1, DWord x2, DWord y2);

public:
    /// @param is -- @c std::istream to read from
    /// @param file_size -- input file size, used only to check header. Set to 0 to disable checks
    BMPReader(std::istream& is, DWord file_size = 0) : is_(&is), file_size_(file_size) {
        // Copy contents of BMP and reset position
        *is_ >> bmp_contents_.rdbuf();
        is_->seekg(0);
    }

    void ReadHeaders() {
        ReadFileHeader();
        ReadInfoHeader();

        if (imp_fields.size_image == 0) {
            imp_fields.size_image = imp_fields.width * imp_fields.height * imp_fields.byte_count;
        }

        if (imp_fields.byte_count != 4) {
            imp_fields.padding_bytes = imp_fields.width * imp_fields.byte_count % 4;
            if (imp_fields.padding_bytes > 0) {
                imp_fields.padding_bytes = 4 - imp_fields.padding_bytes;
            }
        }
    }

    void ReadData();

    void DrawCross(DWord x1, DWord y1, DWord x2, DWord y2) {
        DrawLine(x1, y1, x2, y2);
        DrawLine(x1, y2, x2, y1);
    }

    void SaveBMP(std::string const& filename) {
        std::ofstream ofs{filename};
        ofs << bmp_contents_.rdbuf();
    }

    std::vector<std::vector<bool>> const& GetPixelData() const {
        return pixel_data_;
    }

    // For testing purposes only
    ImportantFields const& GetImportantFields() const {
        return imp_fields;
    }
};

// This operator is useful in tests
inline std::ostream& operator<<(std::ostream& os, BMPReader::ImportantFields const& imp_f) {
    os << "{\n";
    os << "\toffset: " << imp_f.offset << '\n';
    os << "\twidth: " << imp_f.width << '\n';
    os << "\theight: " << imp_f.height << '\n';
    os << "\tbottom-up: " << std::boolalpha << imp_f.bottom_up << '\n';
    os << "\tbyte count: " << imp_f.byte_count << '\n';
    os << "\tcompression method: " << static_cast<Word>(imp_f.compression) << '\n';
    os << "\timage size: " << imp_f.size_image << '\n';
    os << "\tpalette used: " << std::boolalpha << imp_f.palette_used << '\n';
    os << "\tpalette important fields: " << imp_f.palette_important << '\n';
    os << "\tpadding bytes: " << imp_f.padding_bytes << '\n';
    os << "}\n";
    return os;
}

inline bool operator==(BMPReader::ImportantFields const& a, BMPReader::ImportantFields const& b) {
    return a.offset == b.offset && a.width == b.width && a.height == b.height &&
           a.bottom_up == b.bottom_up && a.byte_count == b.byte_count &&
           a.compression == b.compression && a.size_image == b.size_image &&
           a.palette_used == b.palette_used && a.palette_important == b.palette_important &&
           a.padding_bytes == b.padding_bytes;
}
}  // namespace bmp
