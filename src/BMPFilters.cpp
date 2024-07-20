#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include "BMPParser.h"


int main()
{
    std::fstream bmp{ "./img/pinguinos.bmp", std::ios::in | std::ios::out | std::ios::binary };

    // std::array<int, 2> dims{ getImageDimensions(&bmp) };

    pixelArray pixels = loadPixelArray(&bmp);

    std::cout << pixels[0][0][(int) parserConsts::pixelColour_t::BLUE];

    return 0;
}


