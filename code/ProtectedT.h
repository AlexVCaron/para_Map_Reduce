#ifndef PROTECTED_T_H
#define PROTECTED_T_H

#include <mutex>
#include <condition_variable>

template <typename T>
struct protectedT
{
    unsigned t_register = 0, t_registered = 0;
    T t;
    std::mutex m;
    std::condition_variable c_v;
    bool can_transform = false;

    protectedT() {}
    protectedT(T&& t) : t{ t } {}

    void registerOperation()
    {
        std::unique_lock<std::mutex> u_l{ m };
        ++t_register; ++t_registered;
    }
    void allowOperation()
    {
        std::unique_lock<std::mutex> u_l{ m };
        --t_register;
    }

    bool canTransform() const { return can_transform; }
    bool hasPendingTransformations() const { return t_register > 0; }
    unsigned registeredOperations() const { return t_registered; }
    void startTransformations() { can_transform = true; c_v.notify_all(); }
    template <class F, class Arg>
    void waitForTransform(F f, Arg&& arg)
    {
        std::unique_lock<std::mutex> u_l(m);
        if (can_transform) {
            auto f_t = mem_fn(f);
            f_t(t, std::forward<Arg>(arg));
            --t_register;
        }
        else c_v.wait(u_l);
    }
    T getResult()
    {
        using namespace std::literals::chrono_literals;
        unsigned try_outs = 0;
        while (t_register != 0 && try_outs < 5) {
            std::this_thread::sleep_for(1000ms);
            ++try_outs;
        }
        return t;
    }
};


#endif