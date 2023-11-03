/* Copyright (C) 2021 Thomas Friedrich, Chu-Ping Yu, 
 * University of Antwerp - All Rights Reserved. 
 * You may use, distribute and modify
 * this code under the terms of the GPL3 license.
 * You should have received a copy of the GPL3 license with
 * this file. If not, please visit: 
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 * 
 * Authors: 
 *   Thomas Friedrich <thomas.friedrich@uantwerpen.be>
 *   Chu-Ping Yu <chu-ping.yu@uantwerpen.be>
 */

#ifndef ADVAPIX_H
#define ADVAPIX_H

#ifdef __GNUC__
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#endif

#define _USE_MATH_DEFINES
#include <cmath>
#include <atomic>
#include <vector>
#include <array>
#include <thread>
#include <chrono>

#include "FileConnector.h"
#include "Timepix.h.h"

namespace ADVAPIX_ADDITIONAL{
    const size_t buffer_size = 1000*25;
    const size_t n_buffer = 4;
    PACK(struct event
    {
        uint32_t index;
        uint64_t toa;
        uint8_t overflow;
        uint8_t ftoa;
        uint16_t tot;
    });
}

class ADVAPIX : public TIMEPIX
{ };

#endif // ADVAPIX_H