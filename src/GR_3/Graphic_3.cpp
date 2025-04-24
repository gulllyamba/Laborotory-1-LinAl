#include "Graphic_3.hpp"

Graphic_3::Graphic_3(int width, int height) {
    center.x = width / 2;
    center.y = height / 2;
    GenerateGraphic_3();
}

void Graphic_3::GenerateGraphic_3() {
    points.clear();
    faces.clear();
    axisSegments.clear();

    for (int i = 0; i <= uSteps; i++) {
        double u = (2 * PI * i) / uSteps;
        for (int j = 0; j <= vSteps; j++) {
            double v = (2 * PI * j) / vSteps;

            double x = (a + b * cos(v)) * cos(u);
            double y = (a + b * cos(v)) * sin(u);
            double z = b * sin(v);
            points.push_back(Point3D(x, y, z));
        }
    }

    for (int i = 0; i < uSteps; i++) {
        for (int j = 0; j < vSteps; j++) {
            Face face;
            face.indices = {
                i * (vSteps + 1) + j,
                i * (vSteps + 1) + (j + 1),
                (i + 1) * (vSteps + 1) + (j + 1),
                (i + 1) * (vSteps + 1) + j
            };
            faces.push_back(face);
        }
    }

    Point3D origin(0, 0, 0);
    axisSegments.push_back({origin, Point3D(7, 0, 0), 0, 'X', origin, Point3D(7, 0, 0)});
    axisSegments.push_back({origin, Point3D(0, 7, 0), 0, 'Y', origin, Point3D(0, 7, 0)});
    axisSegments.push_back({origin, Point3D(0, 0, -7), 0, 'Z', origin, Point3D(0, 0, -7)});
}

Point3D Graphic_3::RotatePoint(const Point3D& p) {
    double cosX = cos(angleX), sinX = sin(angleX);
    double y1 = p.y * cosX - p.z * sinX;
    double z1 = p.y * sinX + p.z * cosX;

    double cosY = cos(angleY), sinY = sin(angleY);
    double x1 = p.x * cosY + z1 * sinY;
    double z2 = -p.x * sinY + z1 * cosY;

    return Point3D(x1, y1, z2);
}

void Graphic_3::RotateAxes() {
    for (auto& axis : axisSegments) {
        axis.start = RotatePoint(axis.originalStart);
        axis.end = RotatePoint(axis.originalEnd);
    }
}

POINT Graphic_3::Project(const Point3D& p) {
    POINT result;
    result.x = center.x + static_cast<int>(p.x * scale);
    result.y = center.y - static_cast<int>(p.y * scale);
    return result;
}

Point3D Graphic_3::CalculateNormal(const Point3D& p1, const Point3D& p2, const Point3D& p3) {
    Point3D u(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
    Point3D v(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);

    return Point3D(
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x
    );
}

void Graphic_3::Normalize(Point3D& normal) {
    double normalLen = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (normalLen > 0) {
        normal.x /= normalLen;
        normal.y /= normalLen;
        normal.z /= normalLen;
    }
}

bool Graphic_3::IsFaceVisible(const Point3D& p1, const Point3D& p2, const Point3D& p3) {
    Point3D normal = CalculateNormal(p1, p2, p3);
    Normalize(normal);
    Point3D viewDir(0, 0, -1);
    double dot = normal.x * viewDir.x + normal.y * viewDir.y + normal.z * viewDir.z;
    return dot < 0;
}

void Graphic_3::Draw(HDC hdc) {
    std::vector<Point3D> rotatedPoints(points.size());
    for (size_t i = 0; i < points.size(); i++) {
        rotatedPoints[i] = RotatePoint(points[i]);
    }

    RotateAxes();

    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::min();
    for (const auto& point : rotatedPoints) {
        minZ = std::min(minZ, point.z);
        maxZ = std::max(maxZ, point.z);
    }
    for (const auto& axis : axisSegments) {
        minZ = std::min(minZ, axis.start.z);
        minZ = std::min(minZ, axis.end.z);
        maxZ = std::max(maxZ, axis.start.z);
        maxZ = std::max(maxZ, axis.end.z);
    }
    double zRange = maxZ - minZ;

    std::vector<std::pair<double, int>> renderOrder;
    for (size_t i = 0; i < faces.size(); i++) {
        double zSum = 0;
        for (int idx : faces[i].indices) {zSum += rotatedPoints[idx].z;
        }
        faces[i].zDepth = zSum / faces[i].indices.size();
        renderOrder.push_back({faces[i].zDepth, -(int)i});
    }

    for (size_t i = 0; i < axisSegments.size(); i++) {
        axisSegments[i].zDepth = (axisSegments[i].start.z + axisSegments[i].end.z) / 2.0;
        renderOrder.push_back({axisSegments[i].zDepth, (int)i});
    }

    std::sort(renderOrder.begin(), renderOrder.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    HPEN facePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    Point3D rotatedLightPos = RotatePoint(lightPos);
    Normalize(rotatedLightPos);

    for (const auto& item : renderOrder) {
        int index = item.second;
        if (index < 0) {
            index = -index;
            const auto& face = faces[index];
            Point3D p1 = rotatedPoints[face.indices[0]];
            Point3D p2 = rotatedPoints[face.indices[1]];
            Point3D p3 = rotatedPoints[face.indices[2]];
            if (!IsFaceVisible(p1, p2, p3)) {
                continue;
            }

            Point3D normal = CalculateNormal(p1, p2, p3);
            Normalize(normal);
            double lightIntensity = std::max(0.0, normal.x * rotatedLightPos.x +
                                                normal.y * rotatedLightPos.y +
                                                normal.z * rotatedLightPos.z);
            int baseColorValue = static_cast<int>((0.4 + 0.6 * lightIntensity) * 255);

            double fogFactor = (face.zDepth - minZ) / zRange;
            fogFactor = 1.0 - fogFactor;
            int colorValue = static_cast<int>(baseColorValue * (0.5 + 0.5 * fogFactor));
            colorValue = std::min(255, std::max(0, colorValue));
            HBRUSH faceBrush = CreateSolidBrush(RGB(0, 0, colorValue));
            SelectObject(hdc, facePen);
            SelectObject(hdc, faceBrush);

            std::vector<POINT> facePoints;
            for (int idx : face.indices) {
                facePoints.push_back(Project(rotatedPoints[idx]));
            }
            Polygon(hdc, facePoints.data(), facePoints.size());

            DeleteObject(faceBrush);
        } else {
            const auto& axis = axisSegments[index];
            POINT start = Project(axis.start);
            POINT end = Project(axis.end);
            SelectObject(hdc, axisPen);
            MoveToEx(hdc, start.x, start.y, NULL);
            LineTo(hdc, end.x, end.y);
            TextOutA(hdc, end.x, end.y, &axis.label, 1);
        }
    }

    DeleteObject(facePen);
    DeleteObject(axisPen);
}

void Graphic_3::Rotate(double dx, double dy) {
    angleX += dy * 0.01;
    angleY += dx * 0.01;
    RotateAxes();
}

void Graphic_3::ChangeParameters(double newA, double newB) {
    a = newA;
    b = newB;
    GenerateGraphic_3();
}

double Graphic_3::GetScale() const {
    return scale;
}

void Graphic_3::SetScale(double newScale) {
    scale = newScale;
}

double Graphic_3::GetA() {
    return a;
}

double Graphic_3::GetB() {
    return b;
}

void Graphic_3::StartDrag(int x, int y) {
    isDragging = true;
    prevMousePos = {x, y};
}

bool Graphic_3::IsDragging() const {
    return isDragging;
}

void Graphic_3::EndDrag() {
    isDragging = false;
}

void Graphic_3::UpdateDrag(int x, int y) {
    if (isDragging) {
        Rotate(x - prevMousePos.x, y - prevMousePos.y);
        prevMousePos = {x, y};
    }
}