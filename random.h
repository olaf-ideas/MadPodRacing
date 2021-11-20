#ifndef RANDOM_H
#define RANDOM_H

#include <random>

constexpr int FAST_RAND_MAX = 0x7FFF + 1;

inline int fast_rand() {
    static unsigned int g_seed = 2137;

    g_seed = (214013 * g_seed + 2531011); 
    return (g_seed >> 16) & 0x7FFF;
}

int fast_rand(int a, int b) {
    return a + fast_rand() % (b - a + 1);
}

float fast_float_rand() {
    union {
        unsigned int i;
        float f;
    } pun = { 0x3F800000U | (rand() >> 8) };

    return pun.f - 1.0f;
}

float fast_float_rand(float a, float b) {
    return a + fast_float_rand() * (b - a);
}

#endif // RANDOM_H