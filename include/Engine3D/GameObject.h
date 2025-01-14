#ifndef ENGINE3D_GAMEOBJECT_H
#define ENGINE3D_GAMEOBJECT_H

#include "Engine3D/Math.h"

struct triangle {
    vec3d p[3];
    olc::Pixel col;

    // cache calculations that don't change with a new point
    void Barycentric(int32_t x0, int32_t y0, vec3d &a, vec3d &b, vec3d &c,
                     float &u, float &v, float &w) {
        vec3d p = { (float)x0, (float)y0 };
        vec3d v0 = b - a, v1 = c - a, v2 = p - a;
        float d00 = vec3d_DotProduct(v0, v0);
        float d01 = vec3d_DotProduct(v0, v1);
        float d11 = vec3d_DotProduct(v1, v1);
        float d20 = vec3d_DotProduct(v2, v0);
        float d21 = vec3d_DotProduct(v2, v1);
        float denom = d00 * d11 - d01 * d01;
        v = (d11 * d20 - d01 * d21) / denom;
        w = (d00 * d21 - d01 * d20) / denom;
        u = 1.0f - v - w;
    }
};

struct mesh {
    std::vector<triangle> tris;

    bool LoadFromObj(std::string sFileName) {
        std::ifstream f(sFileName);
        if (!f.is_open())
            return false;

        std::vector<vec3d> verts;

        while (!f.eof()) {
            char line[128];
            f.getline(line, 128);

            std::stringstream s;
            s << line;

            char junk;

            if (line[0] == 'v') {
                vec3d v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }
            if (line[0] == 'f') {
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back(
                    { verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
            }
        }

        return true;
    }
};

static int triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n,
                                     triangle &in_tri, triangle &out_tri1,
                                     triangle &out_tri2) {
    plane_n = vec3d_Normalise(plane_n);

    auto dist = [&](vec3d &p) {
        vec3d n = vec3d_Normalise(p);
        return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z -
                vec3d_DotProduct(plane_n, plane_p));
    };

    vec3d *inside_points[3];
    int nInsidePointCount = 0;
    vec3d *outside_points[3];
    int nOutsidePointCount = 0;

    float d0 = dist(in_tri.p[0]);
    float d1 = dist(in_tri.p[1]);
    float d2 = dist(in_tri.p[2]);

    if (d0 >= 0)
        inside_points[nInsidePointCount++] = &in_tri.p[0];
    else
        outside_points[nOutsidePointCount++] = &in_tri.p[0];
    if (d1 >= 0)
        inside_points[nInsidePointCount++] = &in_tri.p[1];
    else
        outside_points[nOutsidePointCount++] = &in_tri.p[1];
    if (d2 >= 0)
        inside_points[nInsidePointCount++] = &in_tri.p[2];
    else
        outside_points[nOutsidePointCount++] = &in_tri.p[2];

    if (nInsidePointCount == 0) {
        return 0;
    }
    if (nInsidePointCount == 3) {
        out_tri1 = in_tri;

        return 1;
    }
    if (nInsidePointCount == 1 && nOutsidePointCount == 2) {
        out_tri1.col = in_tri.col;

        out_tri1.p[0] = *inside_points[0];
        out_tri1.p[1] = vec3d_IntersectPlane(
            plane_p, plane_n, *inside_points[0], *outside_points[0]);
        out_tri1.p[2] = vec3d_IntersectPlane(
            plane_p, plane_n, *inside_points[0], *outside_points[1]);

        return 1;
    }
    if (nInsidePointCount == 2 && nOutsidePointCount == 1) {
        out_tri1.col = in_tri.col;
        out_tri2.col = in_tri.col;

        out_tri1.p[0] = *inside_points[0];
        out_tri1.p[1] = *inside_points[1];
        out_tri1.p[2] = vec3d_IntersectPlane(
            plane_p, plane_n, *inside_points[0], *outside_points[0]);

        out_tri2.p[0] = *inside_points[1];
        out_tri2.p[1] = out_tri1.p[2];
        out_tri2.p[2] = vec3d_IntersectPlane(
            plane_p, plane_n, *inside_points[1], *outside_points[0]);

        return 2;
    }

    return 0;
}

class GameObject {
  public:
    GameObject() {}

  private:
  public:
    vec3d scale = { 1.0f, 1.0f, 1.0f };
    vec3d rotation = { 0.0f, 0.0f, 0.0f };
    vec3d position = { 0.0f, 0.0f, 0.0f };
    mesh *_mesh;
    // texture
};

#endif