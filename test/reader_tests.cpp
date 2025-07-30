#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "bmp_reader.h"
#include "util/field_types.h"
#include "util/ms_constants.h"

using namespace bmp;

namespace test {
constexpr static char kTest1Filename[] = "test_input_data/test1.bmp";
constexpr static char kTest2Filename[] = "test_input_data/test2.bmp";

constexpr static char kTestData[] =
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

struct ReadHeaderParams {
    std::string bmp_filename;
    BMPReader::ImportantFields expected_fields;
};

class ReadHeaderTest : public testing::TestWithParam<ReadHeaderParams> {};

TEST_P(ReadHeaderTest, ReadHeaders) {
    auto const& param = GetParam();

    std::ifstream ifs{param.bmp_filename};
    auto size = std::filesystem::file_size(param.bmp_filename);
    BMPReader reader{ifs, static_cast<bmp::DWord>(size)};
    reader.ReadHeaders();
    auto const& actual_fields = reader.GetImportantFields();

    EXPECT_EQ(actual_fields, param.expected_fields);
}

INSTANTIATE_TEST_SUITE_P(
        ReaderTests, ReadHeaderTest,
        // 24-bit file
        testing::Values(ReadHeaderParams{kTest1Filename,
                                         {

                                                 .offset = 54,
                                                 .width = 10,
                                                 .height = 10,
                                                 .bottom_up = true,
                                                 .byte_count = 3,
                                                 .compression = util::Compression::RGB,
                                                 .size_image = 320,
                                                 .palette_used = false,
                                                 .palette_important = 0,
                                                 .padding_bytes = 2,
                                         }},
                        // 32-bit file (no masks)
                        ReadHeaderParams{kTest2Filename,
                                         {
                                                 .offset = 70,
                                                 .width = 10,
                                                 .height = 10,
                                                 .bottom_up = true,
                                                 .byte_count = 4,
                                                 .compression = util::Compression::BITFIELDS,
                                                 .size_image = 400,
                                                 .palette_used = false,
                                                 .palette_important = 0,
                                                 .padding_bytes = 0,
                                         }}));

struct ReadDataParams {
    std::string bmp_filename;
    std::string expected_data;
};

class ReadDataTest : public testing::TestWithParam<ReadDataParams> {};

TEST_P(ReadDataTest, ReadData) {
    auto const& param = GetParam();
    std::ifstream ifs{param.bmp_filename};
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

    EXPECT_EQ(oss.str(), param.expected_data);
}

INSTANTIATE_TEST_SUITE_P(ReaderTests, ReadDataTest,
                         testing::Values(ReadDataParams{kTest1Filename, kTestData},
                                         ReadDataParams{kTest2Filename, kTestData}));
}  // namespace test
