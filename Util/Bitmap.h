/*
* Copyright (c) 2011 NVIDIA CORPORATION.  All Rights Reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*
*/

#pragma once

#include <windows.h>

// Saves the RGB buffer as a bitmap
bool SaveRGB(const char *fileName, BYTE *data, int width, int height);

// Saves the BGR buffer as a bitmap
bool SaveBGR(const char *fileName, BYTE *data, int width, int height);

// Saves the ARGB buffer as a bitmap
bool SaveARGB(const char *fileName, BYTE *data, int width, int height);

// Saves the RGBPlanar buffer as three bitmaps, one bitmap for each channel
bool SaveRGBPlanar(const char *fileName, BYTE *data, int width, int height);

// Saves the Y'UV420p buffer as three bitmaps, one bitmap for Y', one for U and one for V
bool SaveYUV(const char *fileName, BYTE *data, int width, int height);

// Saves the provided buffer as a bitmap, this method assumes the data is formated as a bitmap.
bool SaveBitmap(const char *fileName, BYTE *data, int width, int height);