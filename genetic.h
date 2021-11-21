#ifndef GENETIC_H
#define GENETIC_H

#include <iostream>

#include "timer.h"
#include "const.h"
#include "arena.h"
#include "random.h"

struct Gen {

    int thrust[GEN_LEN];
    float angle[GEN_LEN];

    void randomize() {
        for(int i = 0; i < GEN_LEN; i++) {
            thrust[i] = fast_rand(0, 400);

            if(thrust[i] > 100) thrust[i] = 100;

            float ang = fast_float_rand(-0.6f, +0.6f);
            if(ang < -MAX_ROTATION_PER_TURN)    ang = -MAX_ROTATION_PER_TURN;
            if(ang > +MAX_ROTATION_PER_TURN)    ang = +MAX_ROTATION_PER_TURN;

            angle[i] = ang;
        }

        if(TURN == 0) {
            thrust[0] = 650;
        }
    }

    void mutate() {

        if(fast_rand() < 7 * FAST_RAND_MAX) {
            thrust[fast_rand() % GEN_LEN] = -1;
        }
        else {
            int i = fast_rand() % GEN_LEN;
            thrust[i] = fast_rand(0, 500);
            if(thrust[i] > 100) {
                thrust[i] = 100;
            }
        }

        int i = fast_rand() % GEN_LEN;

        float ang = fast_float_rand(-1.0f, +1.0f);
        if(ang < -MAX_ROTATION_PER_TURN)    ang = -MAX_ROTATION_PER_TURN;
        if(ang > +MAX_ROTATION_PER_TURN)    ang = +MAX_ROTATION_PER_TURN;

        angle[i] = ang;
    }

    void shift() {
        for(int i = 0; i < GEN_LEN - 1; i++) {
            thrust[i] = thrust[i + 1];
            angle[i] = angle[i + 1];
        }
        thrust[GEN_LEN - 1] = fast_rand(0, 500);
        if(thrust[GEN_LEN - 1] > 100)
            thrust[GEN_LEN - 1] = 100;

        float ang = fast_float_rand(-1.0f, +1.0f);
        if(ang < -MAX_ROTATION_PER_TURN)    ang = -MAX_ROTATION_PER_TURN;
        if(ang > +MAX_ROTATION_PER_TURN)    ang = +MAX_ROTATION_PER_TURN;

        angle[GEN_LEN - 1] = ang;
    }

    void merge(const Gen &a, const Gen &b) {
    
        float x = fast_float_rand();
        for(int i = 0; i < GEN_LEN; i++) {
            thrust[i] = a.thrust[i] * x + b.thrust[i] * (1 - x);
            angle[i]  = a.angle [i] * x + b.angle [i] * (1 - x);
        }

    }

};

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

        for(int turn = 0; turn < GEN_LEN; turn++) {

            for(int i = 0; i < ALL_PODS; i++) {
                arena->pods[i].rotate(gens[i]->angle[turn]);
                
                if(gens[i]->thrust[turn] == -1) {
                    arena->pods[i].shield = 4;
                }
                else {
                    arena->pods[i].thrust(gens[i]->thrust[turn]);
                }
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

        for(int i = 0; i < ALL_PODS; i++) {
            arena->pods[i].save();
        }

        int my_id = player_id * 2;
        int op_id = (1 - player_id) * 2;

        static Gen *gens[4];
        gens[op_id + 0] = &op_gens[0];
        gens[op_id + 1] = &op_gens[1];

        int steps = 0;

        float best_overall = -1e18f;

        while(timer.get_elapsed() < time) {

            float best_score = -1e18f;
            int best_index = -1;
            float nd_score = -1e18f;
            int nd_index = -1;

            float worse_score = +1e18f;
            int worse_index = -1;

            for(int i = 0; i < POP_LEN; i++) {

                gens[my_id + 0] = &pop[i][0];
                gens[my_id + 1] = &pop[i][1];

                if(i > 1) {
                    pop[i][0].mutate();
                    if(fast_rand() & 1) pop[i][0].mutate();
                    pop[i][1].mutate();
                    if(fast_rand() & 1) pop[i][1].mutate();
                }

                float score = play(gens, my_id, op_id);
                
                if(best_index == -1 || best_score < score) {

                    nd_index = best_index;
                    nd_score = best_score;
                    
                    best_index = i;
                    best_score = score;

                }
                else
                if(nd_index == -1 || nd_score < score) {
                    nd_score = score;
                    nd_index = i;
                }

                if(worse_index == -1 || worse_score > score) {
                    worse_index = i;
                    worse_score = score;
                }

            }

            if(worse_index != best_index) {
                if(fast_rand() < 100) {
                    pop[worse_index][0].randomize();
                    pop[worse_index][1].randomize();
                }
                else {
                    pop[worse_index][0].merge(pop[best_index][0], pop[nd_index][0]);
                    pop[worse_index][1].merge(pop[best_index][1], pop[nd_index][1]);
                }
            }

            Gen tmp[PODS_PER_PLAYER];

            tmp[0] = pop[best_index][0];
            tmp[1] = pop[best_index][1];

            pop[best_index][0] = pop[0][0];
            pop[best_index][1] = pop[0][1];

            pop[0][0] = tmp[0];
            pop[0][1] = tmp[1];

            if(best_score > best_overall) {
                best_overall = best_score;
            }

            steps++;

        }

        std::cerr << "best : " << best_overall << '\n';
        std::cerr << "steps: " << steps << '\n';

    }

};


#endif // GENETIC_H