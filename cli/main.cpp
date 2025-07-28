#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "bmp_reader.h"
#include "util/field_types.h"

constexpr static char help[] = R"(BMP Reader
Usage: BMPReader_cli filename x1 y1 x2 y2
OR BMPReader_cli

If arguments are not specified, you will be prompted to enter them.
)";

// Arguments:
// 	- input BMP filename
//	- coordinates of cross (x1, y1, x2, y2)
// If values are not specified in argv, they are read from stdin
int main(int argc, char** argv) {
    std::string filename;
    bmp::DWord x1, y1, x2, y2;
    if (argc == 6) {
        filename = argv[1];
        if (filename == "-h" || filename == "--help") {
            std::cout << help;
            return EXIT_SUCCESS;
        }
        x1 = std::atoi(argv[2]);
        y1 = std::atoi(argv[3]);
        x2 = std::atoi(argv[4]);
        y2 = std::atoi(argv[5]);
    } else {
        std::cout << "Input BMP filename: ";
        std::cin >> filename;
        std::cout << "x1: ";
        std::cin >> x1;
        std::cout << "y1: ";
        std::cin >> y1;
        std::cout << "x2: ";
        std::cin >> x2;
        std::cout << "y2: ";
        std::cin >> y2;
    }

    std::ifstream ifs{filename};
    auto size = std::filesystem::file_size(filename);
    bmp::BMPReader reader{ifs, static_cast<bmp::DWord>(size)};
    reader.ReadHeaders();
    reader.ReadData();

    for (auto const& scan : reader.GetPixelData()) {
        for (auto px : scan) {
            if (px) {
                std::cout << '#';
            } else {
                std::cout << '.';
            }
        }
        std::cout << '\n';
    }
}
