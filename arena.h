#ifndef ARENA_H
#define ARENA_H

#include <iostream>

#include "unit.h"
#include "const.h"

struct Arena {

    Unit pods[ALL_PODS];
    Unit *players[PLAYERS][PODS_PER_PLAYER];

    int laps;

    int checkpoints_count;
    Unit checkpoints[ALL_CHECKPOINTS * MAX_LAPS];

    void init() {
        std::cin >> laps; std::cin.ignore();
        std::cin >> checkpoints_count; std::cin.ignore();
        for(int i = 0; i < checkpoints_count; i++) {
            std::cin >> checkpoints[i].x >> checkpoints[i].y; std::cin.ignore();
        }

        for(int i = checkpoints_count; i < checkpoints_count * (laps + 1); i++) {
            checkpoints[i].x = checkpoints[i - checkpoints_count].x;
            checkpoints[i].y = checkpoints[i - checkpoints_count].y;
        }

        checkpoints_count *= laps;

    }

    void read() {
        
        for(int i = 0; i < ALL_PODS; i++) {
            std::cin >> pods[i].x >> 
                        pods[i].y >> 
                        pods[i].vx >> 
                        pods[i].vy >> 
                        pods[i].ang >> 
                        pods[i].nx_cp; std::cin.ignore();
        }

    }

    void tick() {

        float t = 0.0f;

        for(int i = 0; i < ALL_PODS; i++)
            pods[i].cache();

        while(t < 1.0f) {

            Unit *p, *q;

            float collision_time = 1.0f - t;

            for(int i = 0; i < ALL_PODS; i++) {
                for(int j = i + 1; j < ALL_PODS; j++) {
                    float new_collision_time;

                    if(predict_collision(&pods[i], &pods[j], &new_collision_time)) {
                        if(new_collision_time < collision_time) {
                            p = &pods[i], q = &pods[j];
                            collision_time = new_collision_time;
                        }
                    }

                }
            }
            
            for(int i = 0; i < ALL_PODS; i++)
                pods[i].move(collision_time);

            t += collision_time;

            if(t >= 1.0f)  break;

            simulate_bounce(p, q);

        }

        for(int i = 0; i < ALL_PODS; i++) {
            if(pods[i].nx_cp < checkpoints_count &&
                checkpoint_complete(&pods[i], &checkpoints[pods[i].nx_cp])) {
                if(++pods[i].nx_cp == checkpoints_count) {
                    pods[i].nx_cp = 0;
                }
            }

            pods[i].end_round();
        }

    }

    float score(int my_id, int op_id) {

        float score = 0.0f;

        static float scores[4];

        for(int i = 0; i < ALL_PODS; i++) {
            scores[i] = pods[i].nx_cp * CP_REWARD - distance(&pods[i], &checkpoints[pods[i].nx_cp]);
        }

        int my_runner = scores[my_id] > scores[my_id + 1] ? my_id : my_id + 1;
        int op_runner = scores[op_id] > scores[op_id + 1] ? op_id : op_id + 1;

        int my_blocker = scores[my_id] <= scores[my_id + 1] ? my_id : my_id + 1;
        int op_blocker = scores[op_id] <= scores[op_id + 1] ? op_id : op_id + 1;

        if(pods[op_runner].nx_cp == checkpoints_count) {
            score = -1e9;
        }
        else
        if(pods[my_runner].nx_cp == checkpoints_count) {
            score = +1e9;
        }
        else {
            score = 2 * (scores[my_runner] - scores[op_runner]) 
                    - distance(&pods[my_blocker], &pods[op_runner]) 
                    - distance(&pods[my_blocker], &checkpoints[pods[op_runner].nx_cp]);
        }

        return score;
    }

};

#endif // ARENA_H