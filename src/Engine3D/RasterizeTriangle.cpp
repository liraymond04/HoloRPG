#include "Engine3D/Engine3D.h"
#include "Engine3D/Shaders/Shader.h"

void Engine3D::RasterizeTriangle(triangle &t, olc::Pixel p, Shader *shader) {
    int32_t x1 = t.p[0].x, y1 = t.p[0].y, x2 = t.p[1].x, y2 = t.p[1].y,
            x3 = t.p[2].x, y3 = t.p[2].y;

    auto drawline = [&](int sx, int ex, int ny) {
        float u, v, w;
        for (int i = sx; i <= ex; i++) {
            t.Barycentric(i, ny, t.p[0], t.p[1], t.p[2], u, v, w);
            float z = u * t.p[2].z + v * t.p[1].z + w * t.p[0].z;
            int coord = ny * game->ScreenWidth() + i;
            // bool within = u >= 0.0f && v >= 0.0f && w >= 0.0f;

            // move normalize to vertex shader
            auto map = [](float value, float start1, float stop1, float start2,
                          float stop2) {
                float outgoing = start2 + (stop2 - start2) * ((value - start1) /
                                                              (stop1 - start1));
                return outgoing;
            };
            float val = map(z, 0.99f, 1.0f, 255, 0); // normalize
            if (val > depthBuffer[coord]) {
                game->Draw(i, ny, shader->FragmentShader(p, t, u, v, w));
                depthBuffer[coord] = val;
            }
        }
    };

    int t1x, t2x, y, minx, maxx, t1xp, t2xp;
    bool changed1 = false;
    bool changed2 = false;
    int signx1, signx2, dx1, dy1, dx2, dy2;
    int e1, e2;
    // Sort vertices
    if (y1 > y2) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }
    if (y1 > y3) {
        std::swap(y1, y3);
        std::swap(x1, x3);
    }
    if (y2 > y3) {
        std::swap(y2, y3);
        std::swap(x2, x3);
    }

    t1x = t2x = x1;
    y = y1; // Starting points
    dx1 = (int)(x2 - x1);
    if (dx1 < 0) {
        dx1 = -dx1;
        signx1 = -1;
    } else
        signx1 = 1;
    dy1 = (int)(y2 - y1);

    dx2 = (int)(x3 - x1);
    if (dx2 < 0) {
        dx2 = -dx2;
        signx2 = -1;
    } else
        signx2 = 1;
    dy2 = (int)(y3 - y1);

    if (dy1 > dx1) {
        std::swap(dx1, dy1);
        changed1 = true;
    }
    if (dy2 > dx2) {
        std::swap(dy2, dx2);
        changed2 = true;
    }

    e2 = (int)(dx2 >> 1);
    // Flat top, just process the second half
    if (y1 == y2)
        goto next;
    e1 = (int)(dx1 >> 1);

    for (int i = 0; i < dx1;) {
        t1xp = 0;
        t2xp = 0;
        if (t1x < t2x) {
            minx = t1x;
            maxx = t2x;
        } else {
            minx = t2x;
            maxx = t1x;
        }
        // process first line until y value is about to change
        while (i < dx1) {
            i++;
            e1 += dy1;
            while (e1 >= dx1) {
                e1 -= dx1;
                if (changed1)
                    t1xp = signx1; // t1x += signx1;
                else
                    goto next1;
            }
            if (changed1)
                break;
            else
                t1x += signx1;
        }
        // Move line
    next1:
        // process second line until y value is about to change
        while (1) {
            e2 += dy2;
            while (e2 >= dx2) {
                e2 -= dx2;
                if (changed2)
                    t2xp = signx2; // t2x += signx2;
                else
                    goto next2;
            }
            if (changed2)
                break;
            else
                t2x += signx2;
        }
    next2:
        if (minx > t1x)
            minx = t1x;
        if (minx > t2x)
            minx = t2x;
        if (maxx < t1x)
            maxx = t1x;
        if (maxx < t2x)
            maxx = t2x;
        drawline(minx, maxx, y); // Draw line from min to max points found on
                                 // the y Now increase y
        if (!changed1)
            t1x += signx1;
        t1x += t1xp;
        if (!changed2)
            t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y == y2)
            break;
    }
next:
    // Second half
    dx1 = (int)(x3 - x2);
    if (dx1 < 0) {
        dx1 = -dx1;
        signx1 = -1;
    } else
        signx1 = 1;
    dy1 = (int)(y3 - y2);
    t1x = x2;

    if (dy1 > dx1) { // swap values
        std::swap(dy1, dx1);
        changed1 = true;
    } else
        changed1 = false;

    e1 = (int)(dx1 >> 1);

    for (int i = 0; i <= dx1; i++) {
        t1xp = 0;
        t2xp = 0;
        if (t1x < t2x) {
            minx = t1x;
            maxx = t2x;
        } else {
            minx = t2x;
            maxx = t1x;
        }
        // process first line until y value is about to change
        while (i < dx1) {
            e1 += dy1;
            while (e1 >= dx1) {
                e1 -= dx1;
                if (changed1) {
                    t1xp = signx1;
                    break;
                } // t1x += signx1;
                else
                    goto next3;
            }
            if (changed1)
                break;
            else
                t1x += signx1;
            if (i < dx1)
                i++;
        }
    next3:
        // process second line until y value is about to change
        while (t2x != x3) {
            e2 += dy2;
            while (e2 >= dx2) {
                e2 -= dx2;
                if (changed2)
                    t2xp = signx2;
                else
                    goto next4;
            }
            if (changed2)
                break;
            else
                t2x += signx2;
        }
    next4:

        if (minx > t1x)
            minx = t1x;
        if (minx > t2x)
            minx = t2x;
        if (maxx < t1x)
            maxx = t1x;
        if (maxx < t2x)
            maxx = t2x;
        drawline(minx, maxx, y);
        if (!changed1)
            t1x += signx1;
        t1x += t1xp;
        if (!changed2)
            t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y > y3)
            return;
    }
}