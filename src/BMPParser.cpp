/*
* all the 'magic numbers' that you see in a lot of the 'get' functions are the byte offset positions within the file (relative to the beginning).
* certain information is held at certain byte positions in BMP files.
*
* refer to https://en.wikipedia.org/wiki/BMP_file_format
*/

#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include "BMPParser.h"


int extractLittleEndian(std::fstream* bmp, int startByteOffset, int endByteOffset) {
    int total{ 0 };
    for (int i{ startByteOffset }; i < endByteOffset + 1; ++i) {
        int byte{ bmp->seekg(i, bmp->beg).get() };
        byte = byte << ((i - startByteOffset) * 8);
        total += byte;
    }

    return total;
}

int getFileSize(std::fstream* bmp) {
    /*
    add bytes 02 - 05 with appropriate shifts to get filesize in bytes
    */

    return extractLittleEndian(bmp, 2, 5);
}

int getPixelArrayOffset(std::fstream* bmp) {
    /*
    * add bytes 9 - 12 with appropriate shifts to get location in the file that contains the pixel array
    */

    return extractLittleEndian(bmp, 10, 13);
}

std::array<int, 2> getImageDimensions(std::fstream* bmp) {

    int width{ extractLittleEndian(bmp, 18, 21) };
    int height{ extractLittleEndian(bmp, 22, 25) };

    return std::array<int, 2>{ width, height }; // &dims
}

int getColourDepth(std::fstream* bmp) {
    return extractLittleEndian(bmp, 28, 29);
}

int getDIBHeaderSize(std::fstream* bmp) {
    return extractLittleEndian(bmp, 14, 17);
}

int getNoOfImportantColoursUsed(std::fstream* bmp) {
    return extractLittleEndian(bmp, 50, 53);
}

int getCompressionMethod(std::fstream* bmp) {
    /*
    * 0 - BI_RGB (i.e none)
    * 1 - BI_RLE8 (run-length-encoding, 8 bit)
    * 2 - BI_RLE4 (RLE, 4 bit)
    *
    * there are more on https://en.wikipedia.org/wiki/BMP_file_format,
    * but they're not going to show up because this program only works with BMPs with BITMAPINFOHEADER
    * and other compression methods are either included in different headers or are part of an EMF/WMF file (which is not deal with).
    */

    return extractLittleEndian(bmp, 30, 33);
}

std::array<int, 2> calculateStrideAndSize(std::fstream* bmp) {
    std::array<int, 2> dims{ getImageDimensions(bmp) };
    int width{ dims[0] };
    int height{ dims[1] };

    int bitCount{ getColourDepth(bmp) };

    /*
    * https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
    *
    * (link from which I got stride and size calculations)
    */

    int stride{ ((((width * bitCount) + 31) & ~31) >> 3) };
    int size{ abs(height) * stride };

    return std::array<int, 2>{stride, size};
}
/*
the pixel array starts from the bottom left of the image:  x=0, y= (height - 1)
order of pixel values: BGR
*/
pixelArray loadPixelArray(std::fstream* bmp) {
    std::streamoff offset{ getPixelArrayOffset(bmp) };
    int noBits{ getColourDepth(bmp) };

    std::vector<int> pixels{};

    std::array<int, 2> strideAndSize{ calculateStrideAndSize(bmp) };
    std::array<int, 2> dims{ getImageDimensions(bmp) };
    int width{ dims[0] };
    int height{ dims[1] };

    pixelArray bufferVector{};

    const int bufferSize = strideAndSize[0];
    char* buffer{ new char[bufferSize] };
    bmp->seekg(offset, bmp->beg);
    int index{};
    while (*bmp && index < (strideAndSize[1] / bufferSize)) {
        bmp->read(buffer, bufferSize); // read one stride into buffer

        std::vector<std::vector<uint8_t>> currentStrideVec{};
        std::vector<uint8_t> tempPixel{};

        for (int i{ 0 }; i < bufferSize; ++i) {
            tempPixel.push_back(buffer[i]);

            if ((i + 1) % (noBits / 3) == 0) {
                currentStrideVec.push_back(tempPixel);
                tempPixel = {};
            }
        }

        bufferVector.push_back(currentStrideVec);

        /*if (bmp->fail()) {
            bmp->clear();
        }*/

        std::cout << index << std::endl;
        ++index;
    }

    delete[] buffer;

    std::cout << bufferVector[0].size() << std::endl << std::endl << std::endl << std::endl;
    std::cout << strideAndSize[0] << std::endl;

    return bufferVector;

}

int getTotalHeaderSize(std::fstream* bmp) {
    int BMPHeaderSize{ 14 }; // bytes
    int DIBHeaderSize{ getDIBHeaderSize(bmp) };

    return BMPHeaderSize + DIBHeaderSize;
}

std::vector<std::uint8_t> copyHeaders(std::fstream* bmp) {
    int amountToCopy{ getTotalHeaderSize(bmp) };
    std::vector<std::uint8_t> header{};

    for (int i{ 0 }; i < amountToCopy; ++i) {
        header.push_back((uint8_t)bmp->seekg(i, bmp->beg).get());
    }

    return header;

}

std::vector<std::uint8_t> flattenPixelArray(std::fstream *bmp, pixelArray pixArray) {
    std::vector<std::uint8_t> flattenedMat{};
    std::array<int, 2> dims = getImageDimensions(bmp);
    for (int i{ 0 }; i < dims[1]; ++i) {
        for (int j{ 0 }; j < dims[0]; ++j) {
            for (int k{ 0 }; k < 3; ++k) {
                flattenedMat.push_back(pixArray[i][j][k]);
            }
        }
    }

    return flattenedMat;
}

/* write to a new file */
void writeTransformation(std::fstream* bmp, pixelArray pixArray) {
    std::vector<std::uint8_t> fileVec{};
    std::vector<std::uint8_t> headers{ copyHeaders(bmp) };
    std::vector<std::uint8_t> flattenedPixArr{ flattenPixelArray(bmp, pixArray) };
    
    for (int i{ 0 }; i < headers.size(); ++i) {
        fileVec.push_back(headers[i]);
    }

    for (int j{ 0 }; j < flattenedPixArr.size(); ++j) {
        fileVec.push_back(flattenedPixArr[j]);
    }

};
