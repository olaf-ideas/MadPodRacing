#ifndef GENETIC_H
#define GENETIC_H

#include "timer.h"
#include "const.h"
#include "arena.h"
#include "random.h"

struct Gen {

    int thrust[GEN_LEN];
    float angle[GEN_LEN];

    void randomize() {
        for(int i = 0; i < GEN_LEN; i++) {
            thrust[i] = fast_rand(0, 10) * fast_rand(0, 10);
            angle[i] = fast_float_rand(-MAX_ROTATION_PER_TURN, +MAX_ROTATION_PER_TURN);
        }
    }

    void mutate() {

        if(fast_rand() & 1) {
            thrust[fast_rand() % GEN_LEN] = fast_rand(0, 10) * fast_rand(0, 10);
        }
        else {
            angle[fast_rand() % GEN_LEN] = fast_float_rand(-MAX_ROTATION_PER_TURN, +MAX_ROTATION_PER_TURN);
        }

    }

    void shift() {
        for(int i = 0; i < GEN_LEN; i++) {
            thrust[i] = thrust[i + 1];
            angle[i] = angle[i + 1];
        }
        thrust[GEN_LEN - 1] = fast_rand(0, 10) * fast_rand(0, 10);
        angle[GEN_LEN - 1] = fast_float_rand(-MAX_ROTATION_PER_TURN, +MAX_ROTATION_PER_TURN);
    }

};

float max(float a, float b) { return a > b ? a : b; }

struct Genetic {

    Gen pop[POP_LEN][PODS_PER_PLAYER];
    Arena *arena;

    void init_pop() {
        for(int i = 0; i < POP_LEN; i++) {
            for(int p = 0; p < PODS_PER_PLAYER; p++) {
                pop[i][p].randomize();
            }
        }
    }

    void shift() {
        for(int i = 0; i < POP_LEN; i++) {
            for(int p = 0; p < PODS_PER_PLAYER; p++) {
                pop[i][p].shift();
            }
        }
    }

    float play(Gen **gens, int my_id, int op_id) {
        
        for(int i = 0; i < ALL_PODS; i++) {
            arena->pods[i].save();
        }

        for(int turn = 0; turn < GEN_LEN; turn++) {

            for(int i = 0; i < ALL_PODS; i++) {
                arena->pods[i].rotate(gens[i]->angle[turn]);
                arena->pods[i].thrust(gens[i]->thrust[turn]);
            }

            arena->tick();

        }

        float score = arena->score(my_id, op_id);

        for(int i = 0; i < ALL_PODS; i++) {
            arena->pods[i].load();
        }

        return score;
    }

    void solve(Gen *op_gens, int player_id, float time) {

        int my_id = player_id * 2;
        int op_id = (1 - player_id) * 2;

        static Gen *gens[4];
        gens[op_id + 0] = &op_gens[0];
        gens[op_id + 1] = &op_gens[1];

        while(timer.get_elapsed() < time) {

            float best_score = -1e88;
            int best_index = -1;
            float worse_score = +1e88;
            int worse_index = -1;

            for(int i = 0; i < POP_LEN; i++) {

                gens[my_id + 0] = &pop[i][0];
                gens[my_id + 1] = &pop[i][1];

                if(i > 1) {
                    pop[i][0].mutate();
                    pop[i][1].mutate();
                }

                float score = play(gens, my_id, op_id);
                
                if(best_index == -1 || best_score < score) {
                    best_index = i;
                    best_score = score;
                }

                if(worse_index == -1 || worse_score > score) {
                    worse_index = i;
                    worse_score = score;
                }

            }

            if(worse_index != best_index) {
                pop[worse_index][0].randomize();
                pop[worse_index][1].randomize();
            }

            Gen tmp[PODS_PER_PLAYER];

            tmp[0] = pop[best_index][0];
            tmp[1] = pop[best_index][1];

            pop[best_index][0] = pop[0][0];
            pop[best_index][1] = pop[0][1];

            pop[0][0] = tmp[0];
            pop[0][1] = tmp[1];

        }

    }

};


#endif // GENETIC_H