// *** Start of: /home/olaf/codingame/MadPodRacing/main.cpp *** 
#undef _GLIBCXX_DEBUG                // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

#pragma GCC target("bmi,bmi2,lzcnt,popcnt")                      // bit manipulation
#pragma GCC target("movbe")                                      // byte swap
#pragma GCC target("aes,pclmul,rdrnd")                           // encryption
#pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include <cmath>

 // *** Start of: /home/olaf/codingame/MadPodRacing/genetic.h *** 
 #ifndef GENETIC_H
 #define GENETIC_H
 
 #include <iostream>
 
  // *** Start of: /home/olaf/codingame/MadPodRacing/timer.h *** 
  #ifndef TIMER_H
  #define TIMER_H
  
  #include <chrono>
  
  class Timer {
  
  public:
  	
  	void start() {
  		start_time = std::chrono::steady_clock::now();
  	}
  	
  	inline auto get_elapsed() const {
  		using namespace std::chrono;
  		milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start_time);
  		return elapsed.count();
  	}
  	
  private:
  	
  	std::chrono::steady_clock::time_point start_time;
  
  } timer;
  
  #endif // TIMER_H
  // *** End of: /home/olaf/codingame/MadPodRacing/timer.h *** 
  // *** Start of: /home/olaf/codingame/MadPodRacing/const.h *** 
  #ifndef CONST_H
  #define CONST_H
  
  const float PI = 3.141592653589793f;
  const float TAU = 6.283185307179586f;
  const float EPS = 0.00001f;
  
  const int PLAYERS = 2;
  const int MAX_LAPS = 10;
  const int ALL_PODS = 4;
  const int ALL_CHECKPOINTS = 10;
  const int PODS_PER_PLAYER = 2;
  
  const int WIDTH = 16'000;
  const int HEIGHT = 9000;
  
  const int POD_RADIUS = 400;
  const int CHECKPOINT_RADIUS = 600;
  
  const int MAX_POWER = 200;
  
  const float FRICTION = 0.15f;
  
  const int DEFAULT_POD_TIMEOUT = 100;
  
  const float MAX_ROTATION_PER_TURN = PI / 10;
  
  
  const int GEN_LEN = 4;
  const int POP_LEN = 5;
  
        int TURN = 0;
  
  #endif // CONST_H
  // *** End of: /home/olaf/codingame/MadPodRacing/const.h *** 
  // *** Start of: /home/olaf/codingame/MadPodRacing/arena.h *** 
  #ifndef ARENA_H
  #define ARENA_H
  
  #include <iostream>
  
   // *** Start of: /home/olaf/codingame/MadPodRacing/unit.h *** 
   #ifndef UNIT_H
   #define UNIT_H
   
   #include <cmath>
   
   
   struct Unit {
   
       float  x;
       float  y;
       float vx;
       float vy;
   
       float angle;
   
       int cp_next;
       int cp_pass;
   
       int shield;
       int boosted;
   
       void move(float dt) {
           x += vx * dt;
           y += vy * dt;
       }
   
       void rotate(float by) {
           angle += by;
   
           if(angle < 0)
               angle += TAU;
           if(angle >= TAU)
               angle += TAU;
       }
   
       void thrust(int value) {
           if(shield == 0) {
               vx += cosf(angle) * value;
               vy += sinf(angle) * value;
           }
       }
       
       void end_round() {
            x = roundf(x);
            y = roundf(y);
           vx = int(vx * 0.85f);
           vy = int(vy * 0.85f);
       
           if(shield)  --shield;
       }
   
       float cache[9];
   
       void save() {
           cache[0] = x;
           cache[1] = y;
           cache[2] = vx;
           cache[3] = vy;
           cache[4] = angle;
           cache[5] = cp_next;
           cache[6] = cp_pass;
           cache[7] = shield;
           cache[8] = boosted;
       }
   
       void load() {
           x        = cache[0];
           y        = cache[1];
           vx       = cache[2];
           vy       = cache[3];
           angle    = cache[4];
           cp_next  = cache[5];
           cp_pass  = cache[6];
           shield   = cache[7];
           boosted  = cache[8];
       }
   };
   
   bool predict_collision(const Unit *p, const Unit *q, float *collision_time) {
   
       float dx  = p-> x - q-> x;
       float dy  = p-> y - q-> y;
       float dvx = p->vx - q->vx;
       float dvy = p->vy - q->vy;
   
       // ||P(t) - Q(t)|| = a t^2 + b t + c = R^2
   
       float a = dvx * dvx + dvy * dvy;
   
       if(a == 0.0f)   return false;
   
       float b = dx * dvx + dy * dvy;
       float c = dx * dx + dy * dy - 640000.0f;
   
       float d = b * b - a * c;
   
       if(d <= 0.0f)   return false;
   
       *collision_time = (- b - sqrtf(d)) / a;
   
       return 0 < *collision_time;
   
   }
   
   void simulate_bounce(Unit *p, Unit *q) {
   
       float mp = p->shield == 4 ? 0.1f : 1.0f;
       float mq = q->shield == 4 ? 0.1f : 1.0f;
   
       float dx  = p-> x - q-> x;
       float dy  = p-> y - q-> y;
       float dvx = p->vx - q->vx;
       float dvy = p->vy - q->vy;
   
       float ux = dx * 0.00125f;
       float uy = dy * 0.00125f;
   
       float f = - (dvx * ux + dvy * uy) / (mp + mq);
       
       if(f < 120.0f)  f += 120.f;
       else            f += f;
   
       float ix = ux * f;
       float iy = uy * f;
   
       p->vx += ix * mp;
       p->vy += iy * mp;
   
       q->vx -= ix * mq;
       q->vy -= iy * mq;
   
   }
   
   bool checkpoint_complete(const Unit *pod, const Unit *cp, float dt) {
   
       float ax = pod->x;
       float ay = pod->y;
       float bx = pod->x + pod->vx * dt;
       float by = pod->y + pod->vy * dt;
       float px = cp->x;
       float py = cp->y;
   
       if((bx - ax) * (px - ax) + (by - ay) * (py - ay) > 0.0f &&
          (ax - bx) * (px - bx) + (ay - by) * (py - by) > 0.0f) {
           return ((bx - ax) * (ay - py) - (by - ay) * (ax - px)) * ((bx - ax) * (ay - py) - (by - ay) * (ax - px)) <= 
                  ((ax - bx) * (ax - bx) + (ay - by) * (ay - by)) * 360000.0f;
       }
   
       return (ax - px) * (ax - px) + (ay - py) * (ay - py) <= 360000.0f ||
              (bx - px) * (bx - px) + (by - py) * (by - py) <  360000.0f;
   
   }
   
   float distance(const Unit *p, const Unit *q) {
   
       float dx = p->x - q->x;
       float dy = p->y - q->y;
   
       return sqrtf(dx * dx + dy * dy);
   
   }
   
   float diff_angle(const Unit *p, const Unit *q) {
   
       float angle = atan2(q->y - p->y, q->x - p->x);
   
       float move = angle - p->angle;
   
       if(move < 0)    move += TAU;
       if(move >= TAU) move -= TAU;
   
       float diff = 2 * move;
   
       if(diff >= TAU) diff -= TAU;
   
       diff -= move;
   
       return diff > 0 ? + diff : - diff;
   
   }
   
   
   #endif // UNIT_H
   // *** End of: /home/olaf/codingame/MadPodRacing/unit.h *** 
  
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
                  - 10.0f * diff_angle(&pods[my_runner], &checkpoints[pods[my_runner].cp_next]);
                  
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
  // *** End of: /home/olaf/codingame/MadPodRacing/arena.h *** 
  // *** Start of: /home/olaf/codingame/MadPodRacing/random.h *** 
  #ifndef RANDOM_H
  #define RANDOM_H
  
  #include <random>
  
  constexpr int FAST_RAND_MAX = 0x7FFF + 1;
  
  inline int fast_rand() {
      static unsigned int g_seed = 2137;
  
      g_seed = (214013 * g_seed + 2531011); 
      return (g_seed >> 16) & 0x7FFF;
  }
  
  int fast_rand(int a, int b) {
      return a + fast_rand() % (b - a + 1);
  }
  
  float fast_float_rand() {
      union {
          unsigned int i;
          float f;
      } pun = { 0x3F800000U | (rand() >> 8) };
  
      return pun.f - 1.0f;
  }
  
  float fast_float_rand(float a, float b) {
      return a + fast_float_rand() * (b - a);
  }
  
  #endif // RANDOM_H
  // *** End of: /home/olaf/codingame/MadPodRacing/random.h *** 
 
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
 // *** End of: /home/olaf/codingame/MadPodRacing/genetic.h *** 

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
// *** End of: /home/olaf/codingame/MadPodRacing/main.cpp *** 
