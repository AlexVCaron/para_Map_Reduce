#ifndef METRICS_H
#define METRICS_H

using prot_adder_st = protectedT<adder<size_t>>;

class Metric
{
protected:
    prot_adder_st nb_w_treated;
    time_stamp t_begin, t_end;
public:
    template <class tag, class Arg, class ... Args>
    void beforeTest(Arg&& arg, Args&& ... args) { beforeTest<tag>(arg); beforeTest<tag>(args...); }
    template <class tag, class Arg>
    void beforeTest(Arg&& arg) { beforeTestTag(arg, tag()); }
    template <class tag, class Arg, class ... Args>
    void afterTest(Arg&& arg, Args&& ... args) { afterTest<tag>(arg); afterTest<tag>(args...); }
    template <class tag, class Arg>
    void afterTest(Arg&& arg) { afterTestTag(arg, tag()); }

    time_length getTestDuration() const { return t_end - t_begin; }
    unsigned getNumberWordTreated() { nb_w_treated.startTransformations(); return nb_w_treated.getResult().t; }

private:
    void beforeTestTag(time_stamp& t, parallele_exec_tag) { nb_w_treated.registerOperation(); t_begin = t; }
    void beforeTestTag(time_stamp& t, sequentiel_exec_tag) { nb_w_treated.registerOperation(); nb_w_treated.startTransformations(); t_begin = t; }
    void afterTestTag(time_stamp& t, exec_tag) { t_end = t; }
    void afterTestTag(unsigned nb_words_read, exec_tag)
    {
        nb_w_treated.allowOperation();
        nb_w_treated.waitForTransform(&adder<size_t>::plus, nb_words_read);
    }
};

#endif
