#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "bmp_reader.h"
#include "util/ms_constants.h"

namespace test {
TEST(ReaderTests, ReadHeaders) {
    constexpr static char filename[] = "test_input_data/test1.bmp";

    std::ifstream ifs{filename};
    auto size = std::filesystem::file_size(filename);
    bmp::BMPReader reader{ifs, static_cast<bmp::DWord>(size)};
    reader.ReadHeaders();
    auto const& fields = reader.GetImportantFields();

    EXPECT_EQ(fields.offset, 54);
    EXPECT_EQ(fields.width, 10);
    EXPECT_EQ(fields.height, 10);
    EXPECT_EQ(fields.bottom_up, true);
    EXPECT_EQ(fields.bit_count, 24);
    EXPECT_EQ(fields.compression, bmp::util::Compression::RGB);
    EXPECT_EQ(fields.size_image, 320);
    EXPECT_EQ(fields.palette_used, false);
    EXPECT_EQ(fields.palette_important, 0);
    EXPECT_EQ(fields.padding_bytes, 2);
}

TEST(ReaderTests, Read24bitData) {
    constexpr static char bmp_filename[] = "test_input_data/test1.bmp";
    constexpr static char txt_filename[] = "test_input_data/test1.txt";

    std::ifstream ifs{bmp_filename};
    bmp::BMPReader reader{ifs};
    reader.ReadHeaders();
    reader.ReadData();

    std::ostringstream oss;
    for (auto const& scan : reader.GetPixelData()) {
        for (auto px : scan) {
            if (px) {
                oss << '#';
            } else {
                oss << '.';
            }
        }
        oss << '\n';
    }

    std::string expected =
            ".###..###.\n"
            "..#...#...\n"
            "..#...###.\n"
            "..#...#...\n"
            "..#...###.\n"
            ".###..###.\n"
            ".#.....#..\n"
            ".###...#..\n"
            "...#...#..\n"
            ".###...#..\n";

    EXPECT_EQ(oss.str(), expected);
}
}  // namespace test
