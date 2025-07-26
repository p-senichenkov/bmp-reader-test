#pragma once

// This file contains named constants from Microsoft documentation

namespace bmp::util {
/// @brief BMP Compression method
enum class Compression { RGB = 0, RLE8, RLE4, BITFIELDS, JPEG, PNG, ALPHABITFIELDS };
}  // namespace bmp::util
