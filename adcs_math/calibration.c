#include "calibration.h"

float lowpass_filter(float currValue, float prevValue, float filterConstant) {
    return ((1 - filterConstant) * currValue) + (filterConstant * prevValue);
}

float get_sensor_calibration(float currValue, float prevValue, float offset, float scalar, float filterConstant)
{
    return (lowpass_filter(currValue, prevValue, filterConstant) + offset) * scalar;

}

uint64_t get_delta_t(uint64_t currTime, uint64_t prevTime) {
    if (prevTime <= currTime) {
        return currTime - prevTime;
    } else {
        return (UINT64_MAX - prevTime) + currTime;
    }
}