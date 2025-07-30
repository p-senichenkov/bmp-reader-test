#include <cstdlib>
#include <iostream>
#include <vector>

#include "bmp_reader.h"
#include "util/field_types.h"

constexpr static char help[] = R"(BMP Reader
Usage: BMPReader_cli filename x1 y1 x2 y2 new_filename
OR BMPReader_cli

If arguments are not specified, you will be prompted to enter them.
)";

void PrintPixelData(std::vector<std::vector<bool>> const& data) {
    for (auto const& scan : data) {
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

// Arguments:
// 	- input BMP filename
//	- coordinates of cross (x1, y1, x2, y2)
//	- filename to save edited BMP
// If values are not specified in argv, they are read from stdin
int main(int argc, char** argv) {
    std::string filename, new_filename;
    bmp::DWord x1, y1, x2, y2;
    if (argc == 7) {
        filename = argv[1];
        if (filename == "-h" || filename == "--help") {
            std::cout << help;
            return EXIT_SUCCESS;
        }
        x1 = std::atoi(argv[2]);
        y1 = std::atoi(argv[3]);
        x2 = std::atoi(argv[4]);
        y2 = std::atoi(argv[5]);
        new_filename = argv[6];
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
        std::cout << "Filename to save edited BMP: ";
        std::cin >> new_filename;
    }

    bmp::BMPReader reader{filename};
    reader.ReadHeaders();
    reader.ReadData();

    std::cout << "\t-- Original BMP --\n";
    PrintPixelData(reader.GetPixelData());

    reader.DrawCross(x1, y1, x2, y2);
    std::cout << "\t-- Edited BMP --\n";
    PrintPixelData(reader.GetPixelData());

    reader.SaveBMP(new_filename);
}
