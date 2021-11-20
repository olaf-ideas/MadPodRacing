#undef _GLIBCXX_DEBUG                // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

#pragma GCC target("bmi,bmi2,lzcnt,popcnt")                      // bit manipulation
#pragma GCC target("movbe")                                      // byte swap
#pragma GCC target("aes,pclmul,rdrnd")                           // encryption
#pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include <math.h>

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

    for(int turn = 0; ; turn++) {

        arena.read();

        if(turn == 0) {
            for(int i = 0; i < ALL_PODS; i++) {
                arena.pods[i].ang = atan2(arena.checkpoints[arena.pods[i].nx_cp].y - arena.pods[i].y,
                                          arena.checkpoints[arena.pods[i].nx_cp].x - arena.pods[i].x);
            }
        }

        arena.print();

        timer.start();

        float max_time = turn == 0 ? 1000 : 75;

        op_solver.solve(best_genes, 1, max_time * 0.15f);

        Gen op_gen[2] = {
            op_solver.pop[0][0],
            op_solver.pop[0][1]
        };

        my_solver.solve(op_gen, 0, max_time * 0.95f);

        for(int i = 0; i < PODS_PER_PLAYER; i++) {
            std::cout << int(arena.pods[i].x + 1000 * cos(arena.pods[i].ang + my_solver.pop[0][i].angle[0])) << ' ';
            std::cout << int(arena.pods[i].y + 1000 * sin(arena.pods[i].ang + my_solver.pop[0][i].angle[0])) << ' ';

            if(my_solver.pop[0][i].thrust[0] == -1)
                std::cout << "SHIELD";
            else {
                if(turn == 0)
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