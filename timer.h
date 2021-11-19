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