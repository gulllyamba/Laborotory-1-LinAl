#ifndef POINT3D_HPP
#define POINT3D_HPP

struct Point3D
{
    double x;
    double y;
    double z;
    Point3D() : x(0.0), y(0.0), z(0.0) {}
    Point3D(double x, double y, double z) : x(x), y(y), z(z) {}
};

#endif // POINT3D_HPP