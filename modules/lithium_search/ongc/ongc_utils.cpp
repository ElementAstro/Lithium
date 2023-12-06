#include "ongc_utils.hpp"

#include <cmath>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

double rad2deg(const double &radians)
{
    const double conversionFactor = 180.0 / M_PI;
    return radians * conversionFactor;
}

double deg2rad(const double &degrees)
{
    const double conversionFactor = M_PI / 180.0;
    return degrees * conversionFactor;
}

template<typename T>
T clamp(T value, T min, T max)
{
    if (value < min)
    {
        return min;
    }
    else if (value > max)
    {
        return max;
    }
    else
    {
        return value;
    }
}

template<typename T>
T lerp(T start, T end, double t)
{
    return start + t * (end - start);
}

double smoothstep(double edge0, double edge1, double x)
{
    x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

double normalizeAngle(double angle)
{
    while (angle <= -180.0)
    {
        angle += 360.0;
    }

    while (angle > 180.0)
    {
        angle -= 360.0;
    }

    return angle;
}

double interpolateAngle(double start, double end, double t)
{
    double difference = normalizeAngle(end - start);
    return start + difference * t;
}
