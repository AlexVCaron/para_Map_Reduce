#ifndef PROTECTED_T_H
#define PROTECTED_T_H

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
struct protectedT
{
private:

    T t;

    unsigned t_register = 0, 
             t_registered = 0;
    
    std::mutex m;
    std::condition_variable c_v;

    bool can_transform = false;

public:

    protectedT() {}
    protectedT(const T&& m_other) : t{ m_other } {}

    void anounceFutureOperation()
    {
        std::unique_lock<std::mutex> u_l{ m };
        ++t_register; ++t_registered;
    }

    bool canTransform() const { return can_transform; }
    bool hasPendingOperations() const { return t_register > 0; }

    unsigned registeredOperations() const { return t_registered; }

    void allowOperations() { can_transform = true; c_v.notify_all(); }

    template <class F, class Arg>
    void b_treatOperation(F f, Arg&& arg)
    {
        std::unique_lock<std::mutex> u_l(m);
        if (can_transform) {
            auto f_t = std::bind(f, &t, std::forward<Arg>(arg));
            f_t();
            --t_register;
        }
        else c_v.wait(u_l);
    }

    T getResult()
    {
        return t;
    }
    T b_getResult(unsigned max_t_outs = 5)
    {
        using namespace std::literals::chrono_literals;
        unsigned try_outs = 0;
        while (t_register != 0 && try_outs < max_t_outs) {
            std::this_thread::sleep_for(1000ms);
            ++try_outs;
        }
        return t;
    }
};

#endif