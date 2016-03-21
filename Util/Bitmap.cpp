/*!
 * \copyright
 * Copyright 1993-2014 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users.  These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
*/


#pragma warning(disable : 4995 4996)

#include "Bitmap.h"

#include <stdio.h>
#include <string>

// Macros to help with bitmap padding
#define BITMAP_SIZE(width, height) ((((width) + 3) & ~3) * (height))
#define BITMAP_INDEX(x, y, width) (((y) * (((width) + 3) & ~3)) + (x))

// Describes the structure of a 24-bpp Bitmap pixel
struct BitmapPixel
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
};

// Describes the structure of a RGB pixel
struct RGBPixel
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

// Describes the structure of a ARGB pixel
struct ARGBPixel
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char alpha;
};

bool SaveBitmap(const char *fileName, BYTE *data, int width, int height)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    FILE *outputFile;
    bool bRet = false;

    if (data)
    {
        if(outputFile = fopen(fileName, "wb"))
        {
            width = (width + 3) & (~3);
            int size = width * height * 3; // 24 bits per pixel

            fileHeader.bfType = 0x4D42;
            fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size;
            fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

            infoHeader.biSize = sizeof(BITMAPINFOHEADER);
            infoHeader.biWidth = width;
            infoHeader.biHeight = height;
            infoHeader.biPlanes = 1;
            infoHeader.biBitCount = 24;
            infoHeader.biCompression = BI_RGB;
            infoHeader.biSizeImage = BITMAP_SIZE(width, height);
            infoHeader.biXPelsPerMeter = 0;
            infoHeader.biYPelsPerMeter = 0;
            infoHeader.biClrUsed = 0;
            infoHeader.biClrImportant = 0;

            fwrite((unsigned char *)&fileHeader, 1, sizeof(BITMAPFILEHEADER), outputFile);
            fwrite((unsigned char *)&infoHeader, 1, sizeof(BITMAPINFOHEADER), outputFile);
            fwrite(data, 1, size, outputFile);

            bRet = true;
            fclose(outputFile);
        }
    }

    return bRet;
}

bool SaveRGB(const char *fileName, BYTE *data, int width, int height)
{
    bool result = false;

    RGBPixel *input = (RGBPixel *)data;
    BitmapPixel *output = new BitmapPixel[BITMAP_SIZE(width, height)];

    // Pad bytes need to be set to zero, it's easier to just set the entire chunk of memory
    memset(output, 0, BITMAP_SIZE(width, height) * sizeof(BitmapPixel));

    for(int row = 0; row < height; ++row)
    {
        for(int col = 0; col < width; ++col)
        {
            // In a bitmap (0,0) is at the bottom left, in the frame buffer it is the top left.
            int outputIdx = BITMAP_INDEX(col, row, width);
            int inputIdx = ((height - row - 1) * width) + col;

            output[outputIdx].red = input[inputIdx].red;
            output[outputIdx].green = input[inputIdx].green;
            output[outputIdx].blue = input[inputIdx].blue;
        }
    }

    result = SaveBitmap(fileName, (BYTE *)output, width, height);

    delete [] output;

    return result;
}

bool SaveBGR(const char *fileName, BYTE *data, int width, int height)
{
    bool result = false;

    if (!data)
        return false;
    RGBPixel *input = (RGBPixel *)data;
    BitmapPixel *output = new BitmapPixel[BITMAP_SIZE(width, height)];

    // Pad bytes need to be set to zero, it's easier to just set the entire chunk of memory
    memset(output, 0, BITMAP_SIZE(width, height) * sizeof(BitmapPixel));

    for(int row = 0; row < height; ++row)
    {
        for(int col = 0; col < width; ++col)
        {
            // In a bitmap (0,0) is at the bottom left, in the frame buffer it is the top left.
            int outputIdx = BITMAP_INDEX(col, row, width);
            int inputIdx = ((height - row - 1) * width) + col;

            output[outputIdx].red = input[inputIdx].blue;
            output[outputIdx].green = input[inputIdx].green;
            output[outputIdx].blue = input[inputIdx].red;
        }
    }

    result = SaveBitmap(fileName, (BYTE *)output, width, height);

    delete [] output;

    return result;
}

bool SaveRGBPlanar(const char *fileName, BYTE *data, int width, int height)
{
    if (!data)
        return false;

    const char *nameExt[] = {"red", "green", "blue"};
    BitmapPixel *output = new BitmapPixel[BITMAP_SIZE(width, height)];
    memset(output, 0, BITMAP_SIZE(width, height) * sizeof(BitmapPixel));

    for(int color = 0; color < 3; ++color)
    {
        for(int row = 0; row < height; ++row)
        {
            for(int col = 0; col < width; ++col)
            {   
                int outputIdx = BITMAP_INDEX(col, row, width);
                int inputIdx = ((height - row - 1) * width) + col;

                output[outputIdx].blue = 0;
                output[outputIdx].green = 0;
                output[outputIdx].red = 0;

                switch(color)
                {
                case 0:
                    output[outputIdx].red = data[inputIdx];
                    break;

                case 1:
                    output[outputIdx].green = data[inputIdx + (width * height)];
                    break;

                case 2:
                    output[outputIdx].blue = data[inputIdx + 2 * (width * height)];
                    break;

                default:
                    break;
                }
            }
        }

        std::string outputFile = fileName;
        size_t find = outputFile.find_last_of(".");

        outputFile.insert(find, "-");
        outputFile.insert(find+1, nameExt[color]);

        if(!SaveBitmap(outputFile.c_str(), (BYTE *)output, width, height))
        {
            delete [] output;
            return false;
        }
    }

    delete [] output;

    return true;
}

bool SaveARGB(const char *fileName, BYTE *data, int width, int height)
{
    bool result = false;
    if (!data)
        return result;

    ARGBPixel *input = (ARGBPixel *)data;
    BitmapPixel *output = new BitmapPixel[BITMAP_SIZE(width, height)];
    memset(output, 0, BITMAP_SIZE(width, height) * sizeof(BitmapPixel));

    for(int row = 0; row < height; ++row)
    {
        for(int col = 0; col < width; ++col)
        {
            int outputIdx = BITMAP_INDEX(col, row, width);
            int inputIdx = ((height - row - 1) * width) + col;

            output[outputIdx].red = input[inputIdx].red;
            output[outputIdx].green = input[inputIdx].green;
            output[outputIdx].blue = input[inputIdx].blue;
        }
    }

    result = SaveBitmap(fileName, (BYTE *)output, width, height);

    delete [] output;

    return result;
}

bool SaveYUV(const char *fileName, BYTE *data, int width, int height)
{
    if (!data)
        return false;

    int hWidth = width >> 1;
    int hHeight = height >> 1;
    size_t find = -1;
    std::string outputFile;

    BitmapPixel *luma = new BitmapPixel[BITMAP_SIZE(width, height)];
    BitmapPixel *chrom = new BitmapPixel[BITMAP_SIZE(width, height)];

    memset(luma, 0, BITMAP_SIZE(width, height) * sizeof(BitmapPixel));
    memset(chrom, 0, BITMAP_SIZE(hWidth, hHeight) * sizeof(BitmapPixel));

    for(int row = 0; row < height; ++row)
    {
        for(int col = 0; col < width; ++col)
        {
            int outputIdx = BITMAP_INDEX(col, row, width);
            int inputIdx = ((height - row - 1) * width) + col;

            luma[outputIdx].red = data[inputIdx];
            luma[outputIdx].green = data[inputIdx];
            luma[outputIdx].blue = data[inputIdx];
        }
    }

    data += width * height;

    outputFile = fileName;
    find = outputFile.find_last_of(".");

    outputFile.insert(find, "-");
    outputFile.insert(find+1, "y");

    if(!SaveBitmap(outputFile.c_str(), (BYTE *)luma, width, height))
    {
        delete [] luma;
        delete [] chrom;
        return false;
    }

    for(int row = 0; row < hHeight; ++row)
    {
        for(int col = 0; col < hWidth; ++col)
        {
            int outputIdx = BITMAP_INDEX(col, row, hWidth);
            int inputIdx = ((hHeight - row - 1) * hWidth) + col;

            chrom[outputIdx].red = data[inputIdx];
            chrom[outputIdx].green = 255 - data[inputIdx];
            chrom[outputIdx].blue = 0;
        }
    }

    data += hWidth * hHeight;

    outputFile = fileName;
    find = outputFile.find_last_of(".");

    outputFile.insert(find, "-");
    outputFile.insert(find+1, "u");

    if(!SaveBitmap(outputFile.c_str(), (BYTE *)chrom, hWidth, hHeight))
    {
        delete [] luma;
        delete [] chrom;
        return false;
    }

    for(int row = 0; row < hHeight; ++row)
    {
        for(int col = 0; col < hWidth; ++col)
        {
            int outputIdx = BITMAP_INDEX(col, row, hWidth);
            int inputIdx = ((hHeight - row - 1) * hWidth) + col;

            chrom[outputIdx].red = 0;
            chrom[outputIdx].green = 255 - data[inputIdx];
            chrom[outputIdx].blue = data[inputIdx];
        }
    }

    data += hWidth * hHeight;

    outputFile = fileName;
    find = outputFile.find_last_of(".");

    outputFile.insert(find, "-");
    outputFile.insert(find+1, "v");

    if(!SaveBitmap(outputFile.c_str(), (BYTE *)chrom, hWidth, hHeight))
    {
        delete [] luma;
        delete [] chrom;
        return false;
    }

    delete [] luma;
    delete [] chrom;
    return true;
}
