#ifndef METRICS_H
#define METRICS_H

using prot_adder_st = protectedT<adder<size_t>>;

class Metric
{
    prot_adder_st nb_w_treated;
    time_stamp t_begin, t_end;
public:
    template<class Arg, class ... Args>
    void beforeTest(Arg&& arg, Args&& ... args) { beforeTest(arg); beforeTest(args); }
    template<class Arg, class ... Args>
    void afterTest(Arg&& arg, Args&& ... args) { afterTest(arg); afterTest(args); }

    time_length getTestDuration() const { return t_end - t_begin; }
    unsigned getNumberWordTreated() { nb_w_treated.startTransformations(); return nb_w_treated.getResult().t; }

private:
    void beforeTest(time_stamp& t) { nb_w_treated.registerOperation(); t_begin = t; }
    void afterTest(time_stamp& t) { t_end = t; }
    void afterTest(unsigned nb_words_read)
    {
        nb_w_treated.allowOperation();
        nb_w_treated.waitForTransform(&adder<size_t>::plus, nb_words_read);
    }
};

#endif
