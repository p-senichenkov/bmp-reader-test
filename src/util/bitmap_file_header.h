#pragma once

#include <cstddef>
#include <istream>

#include "util/field_types.h"
#include "util/invalid_bmp_error.h"
#include "util/io_error.h"

namespace bmp::util {
#pragma pack(push, 1)

struct BitmapFileHeader {
    // File signature. Must be "BM"
    Word signature;
    // File size (bytes)
    DWord file_size;
    // Both must be zero
    Word reserved1;
    Word reserved2;
    // Data offset (bytes)
    DWord offset;
};

#pragma pack(pop)

/// @brief Read and check file header
inline std::istream& operator>>(std::istream& is, BitmapFileHeader& file_header) {
    if (!is.read(reinterpret_cast<char*>(&file_header), sizeof(file_header))) {
        throw IOError("cannot read file header");
    }

    // 4D42 is "BM"
    InvalidBMPError::Assert(
            file_header.signature == 0x4D42,
            "invalid magic: must be 4D42, got " + std::to_string(file_header.signature));
    InvalidBMPError::Assert(file_header.file_size > 0, "file size cannot be 0");
    InvalidBMPError::Assert(file_header.reserved1 == 0 && file_header.reserved2 == 0,
                            "one of reserved fields is not zero");
    return is;
}
}  // namespace bmp::util
