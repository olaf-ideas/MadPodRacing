#undef _GLIBCXX_DEBUG                // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

#pragma GCC target("bmi,bmi2,lzcnt,popcnt")                      // bit manipulation
#pragma GCC target("movbe")                                      // byte swap
#pragma GCC target("aes,pclmul,rdrnd")                           // encryption
#pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include <cmath>

#include "genetic.h"

Arena arena;
Genetic my_solver, op_solver;

Gen best_genes[2];

int main() {
    
    arena.init();

    my_solver.arena = &arena;
    op_solver.arena = &arena;

    my_solver.init_pop();
    op_solver.init_pop();

    best_genes[0].randomize();
    best_genes[1].randomize();

    for(; ; TURN++) {

        arena.read();

        if(TURN == 0) {
            for(int i = 0; i < ALL_PODS; i++) {
                arena.pods[i].angle = atan2f(arena.checkpoints[arena.pods[i].cp_next].y - arena.pods[i].y,
                                            arena.checkpoints[arena.pods[i].cp_next].x - arena.pods[i].x);
            }
        }

        arena.print();

        timer.start();

        float max_time = TURN == 0 ? 1000 : 75;

        op_solver.solve(best_genes, 1, max_time * 0.15f);

        Gen op_gen[2] = {
            op_solver.pop[0][0],
            op_solver.pop[0][1]
        };

        my_solver.solve(op_gen, 0, max_time * 0.95f);

        for(int i = 0; i < PODS_PER_PLAYER; i++) {
            std::cout << int(arena.pods[i].x + 5000 * cosf(arena.pods[i].angle + my_solver.pop[0][i].angle[0])) << ' ';
            std::cout << int(arena.pods[i].y + 5000 * sinf(arena.pods[i].angle + my_solver.pop[0][i].angle[0])) << ' ';

            if(my_solver.pop[0][i].thrust[0] == -1)
                std::cout << "SHIELD";
            else {
                if(TURN == 0)
                    std::cout << "BOOST";
                else
                    std::cout << my_solver.pop[0][i].thrust[0];
            }

            std::cout << std::endl;
        }

        op_solver.shift();
        my_solver.shift();

        best_genes[0] = my_solver.pop[0][0];
        best_genes[1] = my_solver.pop[0][1];

    }

}