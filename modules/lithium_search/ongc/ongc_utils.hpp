#pragma once

double rad2deg(const double &radians);
double deg2rad(const double &degrees);
double smoothstep(double edge0, double edge1, double x);
double normalizeAngle(double angle);
double interpolateAngle(double start, double end, double t);