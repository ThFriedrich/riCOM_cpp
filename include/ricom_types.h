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

#ifndef RICOM_TYPES_H_
#define RICOM_TYPES_H_
#include <chrono>

namespace RICOM
{
    enum modes
    {
        FILE,
        LIVE
    };
    enum Detector_type
    {
        MERLIN,
        TIMEPIX
    };
}
#endif // RICOM_TYPES_H_