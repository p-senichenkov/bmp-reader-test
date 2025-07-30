#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

#include "bmp_reader.h"
#include "gtest/gtest.h"
#include "util/field_types.h"

namespace test {
constexpr static char kWhiteFilename[] = "test_input_data/white.bmp";

constexpr static char kCrossOnWhite[] =
        "..........\n"
        "..#.....#.\n"
        "...#...#..\n"
        "....#.#...\n"
        ".....#....\n"
        "....#.#...\n"
        "...#...#..\n"
        "..#.....#.\n"
        "..........\n"
        "..........\n";

struct CrossParams {
    std::string bmp_filename;
    bmp::DWord x1, y1, x2, y2;
    std::string expected_data;
};

class CrossTest : public testing::TestWithParam<CrossParams> {};

TEST_P(CrossTest, DrawCross) {
    auto const& param = GetParam();
    std::ifstream ifs{param.bmp_filename};
    bmp::BMPReader reader{ifs};
    reader.ReadHeaders();
    reader.ReadData();
    reader.DrawCross(param.x1, param.y1, param.x2, param.y2);

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

    EXPECT_EQ(oss.str(), param.expected_data);
}

INSTANTIATE_TEST_SUITE_P(DrawerTests, CrossTest,
                         testing::Values(CrossParams{kWhiteFilename, 2, 2, 8, 8, kCrossOnWhite}));
}  // namespace test
