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

        timer.start();

        float max_time = turn == 0 ? 1000 : 75;

        op_solver.solve(best_genes, 1, max_time * 0.15f);

        Gen op_gen[2] = {
            op_solver.pop[0][0],
            op_solver.pop[0][1]
        };

        my_solver.solve(op_gen, 0, max_time * 0.95f);

        for(int i = 0; i < PODS_PER_PLAYER; i++) {
            std::cerr << int(arena.pods[i].x + 100 * cos(arena.pods[i].ang + my_solver.pop[0][0].angle[0])) << ' ';
            std::cerr << int(arena.pods[i].y + 100 * sin(arena.pods[i].ang + my_solver.pop[0][0].angle[0])) << ' ';
            std::cerr << my_solver.pop[0][0].thrust[0] << std::endl;
        }

        op_solver.shift();
        my_solver.shift();

    }

}