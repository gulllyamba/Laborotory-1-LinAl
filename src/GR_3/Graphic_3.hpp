#ifndef GRAPHIC_3_HPP
#define GRAPHIC_3_HPP

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../Point3D.hpp"

#define PI 3.14159265358979323846

class Graphic_3 {
public:
    Graphic_3(int width, int height);

    void GenerateGraphic_3();

    Point3D RotatePoint(const Point3D& p);

    void RotateAxes();

    POINT Project(const Point3D& p);

    Point3D CalculateNormal(const Point3D& p1, const Point3D& p2, const Point3D& p3);

    void Normalize(Point3D& normal);

    bool IsFaceVisible(const Point3D& p1, const Point3D& p2, const Point3D& p3);

    void Draw(HDC hdc);

    void Rotate(double dx, double dy);

    void ChangeParameters(double newA, double newB);

    double GetScale() const;

    void SetScale(double newScale);

    double GetA();

    double GetB();

    void StartDrag(int x, int y);

    bool IsDragging() const;

    void EndDrag();

    void UpdateDrag(int x, int y);

private:
    struct Face {
        std::vector<int> indices; // Индексы точек грани
        double zDepth; // Средняя Z-координата для сортировки
    };

    struct AxisSegment {
        Point3D start;
        Point3D end;
        double zDepth; // Средняя Z-координата для сортировки
        char label; // Метка оси (X, Y, Z)
        Point3D originalStart; // Исходные координаты
        Point3D originalEnd;
        bool isAxis; // Флаг, указывающий, является ли это сегментом оси
    };
    std::vector<Point3D> points;
    std::vector<Face> faces;
    std::vector<AxisSegment> axisSegments;
    double a = 2.0;
    double b = 0.5;
    double angleX = 0;
    double angleY = 0;
    double scale = 50.0;
    POINT center;
    bool isDragging = false;
    POINT prevMousePos;
    const int uSteps = 30;
    const int vSteps = 30;
    Point3D lightPos = {5.0, 5.0, 5.0};
};

#endif // GRAPHIC_3_HPP