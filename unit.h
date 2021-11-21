#ifndef UNIT_H
#define UNIT_H

#include <cmath>

#include "const.h"

struct Unit {

    float  x;
    float  y;
    float vx;
    float vy;

    float angle;

    int cp_next;
    int cp_pass;

    int shield;
    int boosted;

    void move(float dt) {
        x += vx * dt;
        y += vy * dt;
    }

    void rotate(float by) {
        angle += by;

        if(angle < 0)
            angle += TAU;
        if(angle >= TAU)
            angle += TAU;
    }

    void thrust(int value) {
        if(shield == 0) {
            vx += cosf(angle) * value;
            vy += sinf(angle) * value;
        }
    }
    
    void end_round() {
         x = roundf(x);
         y = roundf(y);
        vx = int(vx * 0.85f);
        vy = int(vy * 0.85f);
    
        if(shield)  --shield;
    }

    float cache[9];

    void save() {
        cache[0] = x;
        cache[1] = y;
        cache[2] = vx;
        cache[3] = vy;
        cache[4] = angle;
        cache[5] = cp_next;
        cache[6] = cp_pass;
        cache[7] = shield;
        cache[8] = boosted;
    }

    void load() {
        x        = cache[0];
        y        = cache[1];
        vx       = cache[2];
        vy       = cache[3];
        angle    = cache[4];
        cp_next  = cache[5];
        cp_pass  = cache[6];
        shield   = cache[7];
        boosted  = cache[8];
    }
};

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

    *collision_time = (- b - sqrtf(d)) / a;

    return 0 < *collision_time;

}

void simulate_bounce(Unit *p, Unit *q) {

    float mp = p->shield == 4 ? 0.1f : 1.0f;
    float mq = q->shield == 4 ? 0.1f : 1.0f;

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

bool checkpoint_complete(const Unit *pod, const Unit *cp, float dt) {

    float ax = pod->x;
    float ay = pod->y;
    float bx = pod->x + pod->vx * dt;
    float by = pod->y + pod->vy * dt;
    float px = cp->x;
    float py = cp->y;

    if((bx - ax) * (px - ax) + (by - ay) * (py - ay) > 0.0f &&
       (ax - bx) * (px - bx) + (ay - by) * (py - by) > 0.0f) {
        return ((bx - ax) * (ay - py) - (by - ay) * (ax - px)) * ((bx - ax) * (ay - py) - (by - ay) * (ax - px)) <= 
               ((ax - bx) * (ax - bx) + (ay - by) * (ay - by)) * 360000.0f;
    }

    return (ax - px) * (ax - px) + (ay - py) * (ay - py) <= 360000.0f ||
           (bx - px) * (bx - px) + (by - py) * (by - py) <  360000.0f;

}

float distance(const Unit *p, const Unit *q) {

    float dx = p->x - q->x;
    float dy = p->y - q->y;

    return sqrtf(dx * dx + dy * dy);

}

float diff_angle(const Unit *p, const Unit *q) {

    float angle = atan2(q->y - p->y, q->x - p->x);

    float move = angle - p->angle;

    if(move < 0)    move += TAU;
    if(move >= TAU) move -= TAU;

    float diff = 2 * move;

    if(diff >= TAU) diff -= TAU;

    diff -= move;

    return diff > 0 ? + diff : - diff;

}


#endif // UNIT_H