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

#include <stdlib.h>
#include <stdio.h>
#include "NvFBC/NvFBC.h"

// Simple macro which checks for NvFBC errors
#define NVFBC_SAFE_CALL(result) nvfbcSafeCall(result, __FILE__, __LINE__)

inline void nvfbcSafeCall(NVFBCRESULT result, const char *file, const int line)
{
    if(result != NVFBC_SUCCESS)
    {
        fprintf(stderr, "NvFBC call failed %s:%d\n", file, line);
        exit(-1);
    }
}
