#pragma once

using pixelArray = std::vector<std::vector<std::vector<uint8_t>>>; // one std::vector for the whole array, one std::vector per row, one std::vector per pixel


namespace parserConsts {
    // for readability
    enum class pixelColour_t : unsigned int {
        BLUE = 0,
        GREEN = 1,
        RED = 2
    };
}


int extractLittleEndian(std::fstream* bmp, int startByteOffset, int endByteOffset);
int getFileSize(std::fstream* bmp);
int getPixelArrayOffset(std::fstream* bmp);
std::array<int, 2> getImageDimensions(std::fstream* bmp);
int getColourDepth(std::fstream* bmp);
int getDIBHeaderSize(std::fstream* bmp);
int getNoOfImportantColoursUsed(std::fstream* bmp);
int getCompressionMethod(std::fstream* bmp);
std::array<int, 2> calculateStrideAndSize(std::fstream* bmp);
pixelArray loadPixelArray(std::fstream* bmp);
int getTotalHeaderSize(std::fstream* bmp);
std::vector<std::uint8_t> copyHeaders(std::fstream* bmp);
std::vector<std::uint8_t> flattenPixelArray(std::fstream *bmp, pixelArray pixArray);
void writeTransformation(std::fstream* bmp, pixelArray pixArray);