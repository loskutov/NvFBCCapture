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

// Simple timer class, measures time in milliseconds
class Timer
{
public:
    // Constructs the timer and starts timing.
    Timer();
    ~Timer();

    // Reset the starting point to now.
    void reset();

    // Get the elapsed milliseconds since the starting point.
    double now();

protected:
    LONGLONG m_llStartTick;
};