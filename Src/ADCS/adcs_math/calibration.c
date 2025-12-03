#include "calibration.h"

float lowpass_filter(float currValue, float prevValue, float filterConstant)
{
    return ((1 - filterConstant) * currValue) + (filterConstant * prevValue);
}

float get_sensor_calibration(float currValue, float prevValue, float offset,
                             float scalar, float filterConstant)
{
    return (lowpass_filter(currValue, prevValue, filterConstant) + offset) *
           scalar;
}

bool calibrateDbl(vi_sensor sensor, double prev, double *curr)
{

    float offset, scalar, filter;

    if (vi_get_sensor_calibration(sensor, &offset, &scalar, &filter))
        return false;

    (*curr) = get_sensor_calibration(*curr, prev, offset, scalar, filter);

    return true;
}

bool calibrateVec3(vi_sensor sensor, vec3 prevVal, vec3 *currVal)
{

    sensor.axis = PX;
    if (calibrateDbl(sensor, prevVal.x, &(currVal->x)))
        return false;
    sensor.axis = PY;
    if (calibrateDbl(sensor, prevVal.y, &(currVal->y)))
        return false;
    sensor.axis = PZ;
    if (calibrateDbl(sensor, prevVal.z, &(currVal->z)))
        return false;

    return true;
}

uint64_t get_delta_t(uint64_t currTime, uint64_t prevTime)
{
    if (prevTime <= currTime) {
        return currTime - prevTime;
    } else {
        return (UINT64_MAX - prevTime) + currTime;
    }
}