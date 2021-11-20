#ifndef ARENA_H
#define ARENA_H

#include <iostream>

#include "unit.h"
#include "const.h"

struct Arena {

    Unit pods[ALL_PODS];

    int laps;
    int checkpoints_count;
    Unit checkpoints[ALL_CHECKPOINTS * MAX_LAPS];

    void init() {
        std::cin >> laps; std::cin.ignore();
        std::cin >> checkpoints_count; std::cin.ignore();
        for(int i = 0; i < checkpoints_count; i++) {
            std::cin >> checkpoints[i].x >> checkpoints[i].y; std::cin.ignore();
        }

        laps++;

        for(int i = checkpoints_count; i < checkpoints_count * laps; i++) {
            checkpoints[i].x = checkpoints[i - checkpoints_count].x;
            checkpoints[i].y = checkpoints[i - checkpoints_count].y;
        }

        checkpoints_count *= laps;

    }

    void read() {
        
        for(int i = 0; i < ALL_PODS; i++) {
            int nx_cp = 0;
            std::cin >> pods[i].x >> 
                        pods[i].y >> 
                        pods[i].vx >> 
                        pods[i].vy >> 
                        pods[i].ang >> 
                        nx_cp; std::cin.ignore();
            
            if(pods[i].nx_cp % (checkpoints_count / laps) != nx_cp) {
                pods[i].nx_cp ++;
            }

            pods[i].ang *= PI / 180.0f;
        }

    }

    void print() {

        for(int i = 0; i < ALL_PODS; i++) {
            std::cerr << "pod : " << i << '\n';
            std::cerr << " x = " << pods[i].x << '\n';
            std::cerr << " y = " << pods[i].y << '\n';
            std::cerr << "vx = " << pods[i].vx << '\n';
            std::cerr << "vy = " << pods[i].vy << '\n';
            std::cerr << "nx = " << pods[i].nx_cp << '\n';
            std::cerr << " a = " << pods[i].ang << '\n';
            std::cerr << " b = " << pods[i].boosted << '\n';
            std::cerr << "----\n";
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
            
            for(int i = 0; i < ALL_PODS; i++) {
                pods[i].move(collision_time);
            }

            t += collision_time;

            if(t >= 1.0f)  break;

            simulate_bounce(p, q);

        }

        for(int i = 0; i < ALL_PODS; i++) {
            pods[i].end_round();
            
            if(pods[i].nx_cp < checkpoints_count &&
                checkpoint_complete(&pods[i], &checkpoints[pods[i].nx_cp])) {
                if(++pods[i].nx_cp == checkpoints_count) {
                    pods[i].nx_cp = 0;
                }
            }
        }

    }

    float score(int my_id, int op_id) {

        float score = 0.0f;

        static float scores[4];

        for(int i = 0; i < ALL_PODS; i++) {
            scores[i] = pods[i].nx_cp * CP_REWARD - distance(&pods[i], &checkpoints[pods[i].nx_cp]);
        }

        int my_runner, op_runner, my_blocker;

        if(scores[my_id] > scores[my_id + 1]) {
            my_runner = my_id;
            my_blocker = my_id + 1;
        }
        else {
            my_runner = my_id + 1;
            my_blocker = my_id;
        }

        if(scores[op_id] > scores[op_id + 1]) {
            op_runner = op_id;
        }
        else {
            op_runner = op_id + 1;
        }

        score = 10 * (scores[my_runner] - scores[op_runner]) 
                - 1.0f * distance(&pods[my_blocker], &pods[op_runner]); 

        return score;
    }

};

#endif // ARENA_H