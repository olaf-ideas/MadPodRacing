#include "../random.h"

#include <iostream>
#include <cassert>

int main() {
    for(int i = 0; i < 1000000; i++) {
        std::cout << fast_float_rand() << '\n';
        assert(0 <= fast_float_rand() && fast_float_rand() < 1);
    }
}