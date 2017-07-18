#ifndef METRICS_H
#define METRICS_H

#include "ProtectedT.h"

using prot_adder_st = protectedT<adder<size_t>>;

class Metric
{
protected:
    prot_adder_st* total_nb_w_treated_ptr = nullptr;
    time_stamp t_begin, t_end;
    unsigned nb_words_treated;
public:
    Metric(): nb_words_treated{ 0 } { }

    Metric(const Metric& m) : t_begin{ m.t_begin }, t_end{ m.t_end }, nb_words_treated{ 0 }, total_nb_w_treated_ptr{ m.total_nb_w_treated_ptr } { }

    void synchroniseAdder(Metric* m) volatile
    {
        total_nb_w_treated_ptr = m->total_nb_w_treated_ptr ;
    }

    void synchroniseAdder(prot_adder_st& pt_s) volatile
    {
        total_nb_w_treated_ptr = &pt_s;
    }



    template <class tag, class Arg, class ... Args>
    void beforeTest(Arg&& arg, Args&& ... args) { beforeTest<tag>(arg); beforeTest<tag>(args...); }
    template <class tag, class Arg>
    void beforeTest(Arg&& arg) { beforeTestTag(arg, tag()); }
    template <class tag, class Arg, class ... Args>
    void afterTest(Arg&& arg, Args&& ... args) { afterTest<tag>(arg); afterTest<tag>(args...); }
    template <class tag, class Arg>
    void afterTest(Arg&& arg) { afterTestTag(arg, tag()); }

    time_length getTestDuration() const { return t_end - t_begin; }
    unsigned getNbWordsTreated() const { return nb_words_treated; }
    
    Metric& operator=(const Metric& m) { total_nb_w_treated_ptr = m.total_nb_w_treated_ptr; t_begin = m.t_begin; t_end = m.t_end; return *this; }
private:
    void beforeTestTag(time_stamp& t, parallele_exec) { total_nb_w_treated_ptr->registerOperation(); t_begin = t; }
    void beforeTestTag(time_stamp& t, sequentiel_exec) { total_nb_w_treated_ptr->registerOperation(); total_nb_w_treated_ptr->startTransformations(); t_begin = t; }
    void afterTestTag(time_stamp& t, exec) { t_end = t; }
    void afterTestTag(unsigned nb_words_read, exec)
    {
        nb_words_treated = nb_words_read;
        total_nb_w_treated_ptr->allowOperation();
        total_nb_w_treated_ptr->waitForTransform(&adder<size_t>::plus, nb_words_read);
    }
};

class GlobalMetric
{
    timer t_timer;

    // Thread metrics
    std::vector<Metric> th_metrics;

    // Global metrics
    time_stamp t_created,
               t_start,
               t_end;

    prot_adder_st total_nb_word_read;

public:
    // Constructors
    GlobalMetric() = delete;
    GlobalMetric(unsigned nb_threads) : th_metrics{ nb_threads }, t_created{ t_timer.now() }, total_nb_word_read{ }
    {
        initThreadsMetrics();
    }

    // Threads metrics accessor
    Metric* getMetricPtr(unsigned i) { return &th_metrics[i]; }

    // Stats setters

    time_stamp registerStart() { t_start = t_timer.now(); return t_start; }
    time_stamp registerEnd() { t_end = t_timer.now(); return t_end; }

    // Stats accessors
    //
    // Beware, if there is some remaining threads on work that have still not allowed their nb_words_treated to be added to the total
    // the method getNumberWordTreated() bellow will block execution in the present thread, unless calculateNumberWordTreated() has been
    // called before and processing is done, as blocking in this case is done on calculateNumberWordTreated().
    //
    // The other methods are non-blocking and should be used only when threads have finished working, except for
    // getNbThreads() and getCreationTime(), which are available right after creation.

    unsigned getNbThreads() const { return th_metrics.size(); }

    time_stamp getCreationTime() const { return t_created; }

    unsigned getNumberWordTreated() { total_nb_word_read.startTransformations(); return total_nb_word_read.getResult().t; }

    void calculateNumberWordTreated() { total_nb_word_read.startTransformations(); }

    std::vector<unsigned> getIndividualNumberWordTreated()
    {
        std::vector<unsigned> i_nb_w_t; for_each(th_metrics.begin(), th_metrics.end(), [&](Metric& m)
        {
            i_nb_w_t.push_back(m.getNbWordsTreated());
        });
        return i_nb_w_t;
    }

    std::vector<time_length> getThreadsWorkTime() {
        std::vector<time_length> th_time; std::for_each(th_metrics.begin(), th_metrics.end(), [&](Metric& th_m)
        {
            th_time.push_back(th_m.getTestDuration());
        });
        return th_time;
    }

    time_length getDuration() const { return t_end - t_start; }

private:
    void initThreadsMetrics()
    {
        std::for_each(th_metrics.begin(), th_metrics.end(), [&] (Metric& m)
        {
            m.synchroniseAdder(total_nb_word_read);
        });
    }
};

#endif
