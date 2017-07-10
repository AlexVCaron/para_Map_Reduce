#ifndef STREAM_MONITOR_H
#define STREAM_MONITOR_H

#include <atomic>
#include <mutex>

template<class flux>
class monitor {
    std::atomic<bool> is_available;
    std::mutex m;
    flux f;
    monitor() { }
    template<class Arg>
    void print_r(Arg&& arg) {
        f << arg;
    }
    template<class Arg, class ... Args>
    void print_r(Arg&& arg1, Args&& args) {
        print_r(arg1);
        print_r(args);
    }
public:
    monitor(flux&& f) : f{ f } { }
    template<class ... Args>
    void print(Args&& args)
    {
        std::unique_lock<std::mutex> ul_m(m);
        print_r(args);
    }
};

#endif