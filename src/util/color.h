#pragma once

#include "util/field_types.h"

namespace bmp::util {
#pragma pack(push, 1)

/// @brief 24-bit RGB color
/// @note BMP uses non-standard format: BGR
struct RGBColor {
    Byte blue;
    Byte green;
    Byte red;
};

#pragma pack(pop)
}  // namespace bmp::util
