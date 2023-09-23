#include "camera_utils.hpp"

bool isValidISOValue(int isoValue, int minISO, int maxISO, int isoStep)
{
    if (isoValue < minISO || isoValue > maxISO)
    {
        return false;
    }
    if ((isoValue - minISO) % isoStep != 0)
    {
        return false;
    }
    return true;
}
/*
int isoValue = 800;  // 假设设置ISO值为800
int minISO = 100;   // 假设最小ISO值为100
int maxISO = 6400;  // 假设最大ISO值为6400
int isoStep = 100;  // 假设ISO调整步长为100

bool isValid = IsValidISOValue(isoValue, minISO, maxISO, isoStep);
if (isValid)
{
    std::cout << "ISO " << isoValue << " is valid." << std::endl;
}
else
{
    std::cout << "ISO " << isoValue << " is not valid." << std::endl;
}
*/