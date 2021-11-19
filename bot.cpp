// *** Start of: /home/olaf/codingame/MadPodRacing/main.cpp *** 
#include <math.h>

 // *** Start of: /home/olaf/codingame/MadPodRacing/genetic.h *** 
 #ifndef GENETIC_H
 #define GENETIC_H
 
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
  const float EPS = 1E-5f;
  
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
  
  
  const int GEN_LEN = 5;
  const int POP_LEN = 128;
  const int CP_REWARD = 30'000;
  
  #endif // CONST_H
  // *** End of: /home/olaf/codingame/MadPodRacing/const.h *** 
  // *** Start of: /home/olaf/codingame/MadPodRacing/arena.h *** 
  #ifndef ARENA_H
  #define ARENA_H
  
  #include <iostream>
  
   // *** Start of: /home/olaf/codingame/MadPodRacing/unit.h *** 
   #ifndef UNIT_H
   #define UNIT_H
   
   #include <math.h>
   
   struct Unit {
   
       float  x;
       float  y;
       float vx;
       float vy;
   
       float ang;
   
       int nx_cp;
   
       int shield;
       bool boosted;
   
       float px;
       float py;
   
       float backup[8];
   
       void cache() {
           px = x;
           py = y;
       }
   
       void save() {
           backup[0] = x;
           backup[1] = y;
           backup[2] = vx;
           backup[3] = vy;
           backup[4] = ang;
           backup[5] = shield;
           backup[6] = boosted;
       }
   
       void load() {
           x = backup[0];
           y = backup[1];
           vx = backup[2];
           vy = backup[3];
           ang = backup[4];
           shield = backup[5];
           boosted = backup[6];
       }
   
       void move(float t) {
           x += vx * t;
           y += vy * t;
       }
   
       void rotate(float new_ang) {
           ang += new_ang;
   
           if(ang < 0)     ang += TAU;
           if(ang >= TAU)  ang -= TAU;
       }
   
       void thrust(int t) {
           if(shield == 0) {
               vx += cos(ang) * t;
               vy += sin(ang) * t;
           }
       }
       
       void end_round() {
            x = int(x + 0.5f);
            y = int(y + 0.5f);
           vx = int(vx * 0.85f);
           vy = int(vy * 0.85f);
       
           if(shield)  --shield;
       }
   };
   
   bool predict_collision(const Unit *p, const Unit *q, float *collision_time) {
   
       float dx  = p-> x - q-> x;
       float dy  = p-> y - q-> y;
       float dvx = p->vx - q->vx;
       float dvy = p->vy - q->vy;
   
       // ||P(t) - Q(t)|| = a t^2 + b t + c = R^2
   
       float a = dvx * dvx + dvy * dvy;
   
       if(a < EPS)  return false;
   
       float b = dx * dvx + dy * dvy;
       float c = dx * dx + dy * dy - 640000.0f;
   
       float d = b * b - a * c;
   
       if(d <= 0.0f)   return false;
   
       *collision_time = (- b - sqrt(d)) / a;
   
       return EPS < *collision_time;
   
   }
   
   void simulate_bounce(Unit *p, Unit *q) {
   
       float mp = p->shield ? 0.1f : 1.0f;
       float mq = q->shield ? 0.1f : 1.0f;
   
       float dx  = p-> x - q-> x;
       float dy  = p-> y - q-> y;
       float dvx = p->vx - q->vx;
       float dvy = p->vy - q->vy;
   
       float ux = dx * 0.00125f;
       float uy = dy * 0.00125f;
   
       float f = - (dvx * ux + dvy * uy) / (mp + mq);
       
       if(f < 120.0f)  f += 120.f;
       else            f += f;
   
       int ix = ux * f;
       int iy = uy * f;
   
       p->vx += ix * mp;
       p->vy += iy * mp;
   
       q->vx -= ix * mq;
       q->vy -= iy * mq;
   
   }
   
   bool checkpoint_complete(const Unit *pod, const Unit *cp) {
   
       float dpx = pod->px - pod->x;
       float dpy = pod->py - pod->y;
   
       float d1x = cp->x - pod->x;
       float d1y = cp->y - pod->y;
   
       float d2x = cp->x - pod->px;
       float d2y = cp->y - pod->py;
   
       float d = dpx * dpx + dpy * dpy;
   
       if(d1x * d1x + d1y * d1y <= 1000000.0f &&
          d2x * d2x + d2y * d2y <= 1000000.0f)
           return true;
   
       if(d == 0)
           return false;
   
       float t = (d1x * dpx + d1y * dpy) / d;
       float x = d1x + t * dpx;
       float y = d1y + t * dpy;
   
       return x * x + y * y <= 1000000.0f;
   }
   
   float distance(const Unit *p, const Unit *q) {
   
       float dx = p->x - q->x;
       float dy = p->y - q->y;
   
       return sqrt(dx * dx + dy * dy);
   
   }
   
   
   #endif // UNIT_H
   // *** End of: /home/olaf/codingame/MadPodRacing/unit.h *** 
  
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
  // *** End of: /home/olaf/codingame/MadPodRacing/arena.h *** 
  // *** Start of: /home/olaf/codingame/MadPodRacing/random.h *** 
  #ifndef RANDOM_H
  #define RANDOM_H
  
  #include <random>
  
  const int FAST_RAND_MAX = RAND_MAX;
  
  int fast_rand() { // speed it up
      return rand();
  }
  
  int fast_rand(int a, int b) {
      return a + fast_rand() % (b - a + 1);
  }
  
  float fast_float_rand() {
      return fast_rand() / float(FAST_RAND_MAX);
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
                 gens[my_id * 2 + 1] = &pop[i][1];
 
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
            std::cerr << arena.pods[i].x + cos(arena.pods[i].ang + my_solver.pop[0][0].angle[0]) << ' ';
            std::cerr << arena.pods[i].y + sin(arena.pods[i].ang + my_solver.pop[0][0].angle[0]) << ' ';
            std::cerr << my_solver.pop[0][0].thrust[0] << std::endl;
        }

        op_solver.shift();
        my_solver.shift();

    }

}
// *** End of: /home/olaf/codingame/MadPodRacing/main.cpp *** 
