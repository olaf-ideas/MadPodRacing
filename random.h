#ifndef RANDOM_H
#define RANDOM_H

#include <random>

const int FAST_RAND_MAX = RAND_MAX;

int fast_rand() { // speed it up
    return rand();
}

int fast_rand(int a, int b) {
    return a + fast_rand() % (b - a + 1);
}

float fast_float_rand() {
    return fast_rand() / float(FAST_RAND_MAX);
}

float fast_float_rand(float a, float b) {
    return a + fast_float_rand() * (b - a);
}

#endif // RANDOM_H