#ifndef UNIT_H
#define UNIT_H

#include <math.h>

#include "const.h"

struct Unit {

    float  x;
    float  y;
    float vx;
    float vy;

    float ang;

    int nx_cp;

    int shield;
    int boosted;

    float px;
    float py;

    float backup[8];

    void cache() {
        px = x;
        py = y;
    }

    void save() {
        backup[0] = x;
        backup[1] = y;
        backup[2] = vx;
        backup[3] = vy;
        backup[4] = ang;
        backup[5] = shield;
        backup[6] = boosted;
    }

    void load() {
        x = backup[0];
        y = backup[1];
        vx = backup[2];
        vy = backup[3];
        ang = backup[4];
        shield = backup[5];
        boosted = backup[6];
    }

    void move(float t) {
        x += vx * t;
        y += vy * t;
    }

    void rotate(float new_ang) {
        ang += new_ang;

        if(ang < 0)     ang += TAU;
        if(ang >= TAU)  ang -= TAU;
    }

    void thrust(int t) {
        if(shield == 0) {
            vx += cos(ang) * t;
            vy += sin(ang) * t;
        }
    }
    
    void end_round() {
         x = int(x + 0.5f);
         y = int(y + 0.5f);
        vx = int(vx * 0.85f);
        vy = int(vy * 0.85f);
    
        if(shield)  --shield;
    }
};

bool collide(const Unit *p, const Unit *q) {

    float dx = p->x - q->x;
    float dy = p->y - q->y;
    
    return dx * dx + dy * dy <= 640000.0f;

}

bool predict_collision(const Unit *p, const Unit *q, float *collision_time) {

    float dx  = p-> x - q-> x;
    float dy  = p-> y - q-> y;
    float dvx = p->vx - q->vx;
    float dvy = p->vy - q->vy;

    // ||P(t) - Q(t)|| = a t^2 + b t + c = R^2

    float a = dvx * dvx + dvy * dvy;

    if(a == 0.0f)   return false;

    float b = dx * dvx + dy * dvy;
    float c = dx * dx + dy * dy - 640000.0f;

    float d = b * b - a * c;

    if(d <= 0.0f)   return false;

    *collision_time = (- b - sqrt(d)) / a;

    return 0 < *collision_time;

}

void simulate_bounce(Unit *p, Unit *q) {

    float mp = p->shield ? 0.1f : 1.0f;
    float mq = q->shield ? 0.1f : 1.0f;

    float dx  = p-> x - q-> x;
    float dy  = p-> y - q-> y;
    float dvx = p->vx - q->vx;
    float dvy = p->vy - q->vy;

    float ux = dx * 0.00125f;
    float uy = dy * 0.00125f;

    float f = - (dvx * ux + dvy * uy) / (mp + mq);
    
    if(f < 120.0f)  f += 120.f;
    else            f += f;

    float ix = ux * f;
    float iy = uy * f;

    p->vx += ix * mp;
    p->vy += iy * mp;

    q->vx -= ix * mq;
    q->vy -= iy * mq;

}

bool checkpoint_complete(const Unit *pod, const Unit *cp) {

    float dpx = pod->px - pod->x;
    float dpy = pod->py - pod->y;

    float d1x = cp->x - pod->x;
    float d1y = cp->y - pod->y;

    float d2x = cp->x - pod->px;
    float d2y = cp->y - pod->py;

    float d = dpx * dpx + dpy * dpy;

    if(d1x * d1x + d1y * d1y <= 1000000.0f &&
       d2x * d2x + d2y * d2y <= 1000000.0f)
        return true;

    if(d == 0)
        return false;

    float t = (d1x * dpx + d1y * dpy) / d;
    float x = d1x + t * dpx;
    float y = d1y + t * dpy;

    return x * x + y * y <= 1000000.0f;
}

float distance(const Unit *p, const Unit *q) {

    float dx = p->x - q->x;
    float dy = p->y - q->y;

    return sqrt(dx * dx + dy * dy);

}


#endif // UNIT_H