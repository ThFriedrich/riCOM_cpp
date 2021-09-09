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

    typedef std::chrono::duration<float, std::milli> double_ms;
}
#endif // RICOM_TYPES_H_