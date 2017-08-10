#ifndef TIMER_H
#define TIMER_H
#include <functional>
#include <chrono>

struct Timer {
        Timer(std::function<void(long long)> callback)
                : callback(callback)
                , t0(std::chrono::high_resolution_clock::now()) { ; }
        ~Timer(void) {
                auto t1 = std::chrono::high_resolution_clock::now();
                long long elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
                callback(elapsed);
        }
        std::function<void(long long)> callback;
        std::chrono::high_resolution_clock::time_point t0;

};

#endif
