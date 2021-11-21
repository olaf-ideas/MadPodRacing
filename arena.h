#ifndef ARENA_H
#define ARENA_H

#include <iostream>

#include "unit.h"
#include "const.h"

struct Arena {

    Unit pods[ALL_PODS];

    int laps;
    int checkpoints_count;
    
    Unit checkpoints[ALL_CHECKPOINTS];

    void tick() {

        float t = 0.0f;

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
                if(checkpoint_complete(&pods[i], &checkpoints[pods[i].cp_next], collision_time)) {
                    if(++pods[i].cp_next == checkpoints_count)
                        pods[i].cp_next = 0;
                    pods[i].cp_pass ++;
                }
                pods[i].move(collision_time);
            }

            t += collision_time;

            if(t == 1.0f)    break;

            simulate_bounce(p, q);

        }

        for(int i = 0; i < ALL_PODS; i++) {
            pods[i].end_round();
        }

    }

    float score(int my_id, int op_id) {

        float score = 0.0f;

        static float scores[4];

        for(int i = 0; i < ALL_PODS; i++) {
            scores[i] = pods[i].cp_pass * 100'000 - distance(&pods[i], &checkpoints[pods[i].cp_next]);
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

        if(pods[my_runner].cp_pass == checkpoints_count * laps + 1) {
            return +1e8;
        }
        else
        if(pods[op_runner].cp_pass == checkpoints_count * laps + 1) {
            return -1e8;
        }
        score = + 10 * (scores[my_runner] - scores[op_runner]) 
                - 2.0f    * distance(&pods[my_blocker], &pods[op_runner])
                - 1.0f    * distance(&pods[op_runner], &checkpoints[pods[op_runner].cp_next])
                - 10.0f   * diff_angle(&pods[my_runner], &checkpoints[pods[my_runner].cp_next]);

        return score;

    }

    void init() {

        std::cin >> laps; std::cin.ignore();
        std::cin >> checkpoints_count; std::cin.ignore();
        
        for(int i = 0; i < checkpoints_count; i++) {
            std::cin >> checkpoints[i].x >> checkpoints[i].y; std::cin.ignore();
        }

    }

    void read() {
        
        for(int i = 0; i < ALL_PODS; i++) {

            int cp_next;

            std::cin >> pods[i].x >>
                        pods[i].y >>
                        pods[i].vx >>
                        pods[i].vy >> 
                        pods[i].angle >>
                        cp_next;

            std::cin.ignore();

            if(cp_next != pods[i].cp_next) {
                pods[i].cp_next = cp_next;
                pods[i].cp_pass ++;
            }

            pods[i].angle *= PI / 180.0f;
        }

    }

    void print() {

        for(int i = 0; i < ALL_PODS; i++) {

            std::cerr << "id = " << i << '\n';
            std::cerr << " x = " << pods[i].x << '\n';
            std::cerr << " y = " << pods[i].y << '\n';
            std::cerr << "vx = " << pods[i].vx << '\n';
            std::cerr << "vy = " << pods[i].vy << '\n';
            std::cerr << "nx = " << pods[i].cp_next << '\n';
            std::cerr << "pa = " << pods[i].cp_pass << '\n';
            std::cerr << " a = " << pods[i].angle << '\n';
            std::cerr << " b = " << pods[i].boosted << '\n';
            std::cerr << "----\n";
        
        }

    }

};

#endif // ARENA_H