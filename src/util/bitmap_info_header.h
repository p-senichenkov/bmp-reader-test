#pragma once

#include <istream>

#include "util/field_types.h"
#include "util/invalid_bmp_error.h"
#include "util/io_error.h"
#include "util/ms_constants.h"

// BMP may contain different types of info headers: CORE (16-bit) and several 32-bit variants.
// Newer 32-bit versions contain a lot of fields that are useless here.
// Size of info header is saved in a separate field of type DWord.

namespace bmp::util {
#pragma pack(push, 1)

/// @brief Old (16-bit) info header
struct BitmapCoreHeader {
    // Width and height (px)
    Word width;
    Word height;
    // Must be 1
    Word planes;
    // Bits per pixel
    Word bit_count;
};

#pragma pack(pop)

inline std::istream& operator>>(std::istream& is, BitmapCoreHeader& core_header) {
    if (!is.read(reinterpret_cast<char*>(&core_header), sizeof(core_header))) {
        throw IOError("cannot read CORE info header");
    }

    InvalidBMPError::Assert(core_header.width > 0 && core_header.height > 0,
                            "width and height must be positive");
    InvalidBMPError::Assert(core_header.planes == 1,
                            "plains must be 1, got " + std::to_string(core_header.planes));
    // 32-bit CORE is not documented by Microsoft, but isn't impossible
    InvalidBMPError::Assert(core_header.bit_count == 24 || core_header.bit_count == 32,
                            std::to_string(core_header.bit_count) + "-bit BMPs are not supported");
    return is;
}

#pragma pack(push, 1)

/// @brief New (32-bit) info header.
/// This is version 3 header; versions 4 and 5 contain some additional fields.
struct BitmapInfoHeader {
    // Width (must be positive)
    Long width;
    // Height (absolute value) and scans order (sign)
    Long height;
    // Must be 1
    Word planes;
    // Bits per pixel
    Word bit_count;
    // Compression (see Compression enum)
    DWord compression;
    // Data size (bytes). Can be zero when data is not compressed.
    DWord size_image;
    // Image resolution
    Long x_px_per_meter;
    Long y_px_per_meter;
    // Pallette size (records)
    DWord clr_used;
    // Number of used pallette records
    DWord clr_important;
};

#pragma pack(pop)

inline std::istream& operator>>(std::istream& is, BitmapInfoHeader& info_header) {
    if (!is.read(reinterpret_cast<char*>(&info_header), sizeof(info_header))) {
        throw IOError("cannot read info header");
    }

    InvalidBMPError::Assert(info_header.width > 0, "width must be positive");
    InvalidBMPError::Assert(info_header.height != 0, "height cannot be zero");
    InvalidBMPError::Assert(info_header.planes == 1,
                            "planes must be 1, got " + std::to_string(info_header.planes));
    InvalidBMPError::Assert(info_header.bit_count == 24 || info_header.bit_count == 32,
                            std::to_string(info_header.bit_count) + "-bit BMPs are not supported");
    InvalidBMPError::Assert(
            info_header.compression == static_cast<DWord>(Compression::RGB) ||
                    info_header.compression == static_cast<DWord>(Compression::BITFIELDS) ||
                    info_header.compression == static_cast<DWord>(Compression::ALPHABITFIELDS),
            "invalid compression for 24 or 32-bit BMPs: " +
                    std::to_string(info_header.compression));
    InvalidBMPError::Assert(info_header.compression == static_cast<DWord>(Compression::RGB) ||
                                    info_header.size_image > 0,
                            "size image cannot be 0 when compression is not RGB");
    // Image resolution is ignored
    InvalidBMPError::Assert(
            info_header.clr_important <= info_header.clr_used,
            "number of important fields in palette must not be greater than number of all fields");
    return is;
}
}  // namespace bmp::util
