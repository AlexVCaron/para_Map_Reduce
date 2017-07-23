#ifndef METRICS_H
#define METRICS_H

#include "types.h"
#include "MRUtilities.h"
#include "ProtectedT.h"

using prot_adder_st = protectedT<adder<size_t>>;

class Metric
{

protected:

    prot_adder_st* total_nb_w_treated_ptr = nullptr;
    unsigned inner_nb_words_treated;
    time_stamp t_begin, t_end;
    
public:

    Metric(): inner_nb_words_treated{ 0 } { }
    Metric(const Metric& other) : total_nb_w_treated_ptr{ other.total_nb_w_treated_ptr }, inner_nb_words_treated{ 0 }, t_begin{ other.t_begin }, t_end{ other.t_end } { }

    void synchroniseAdder(Metric* m) volatile
    {
        total_nb_w_treated_ptr = m->total_nb_w_treated_ptr ;
    }

    void synchroniseAdder(prot_adder_st& pt_s) volatile
    {
        total_nb_w_treated_ptr = &pt_s;
    }

    time_length getTestDuration() const { return t_end - t_begin; }
    unsigned getNbWordsTreated() const { return inner_nb_words_treated; }

    template <class tag, class Arg, class ... Args>
    void beforeTest(Arg&& arg, Args&& ... args) { beforeTest<tag>(arg); beforeTest<tag>(args...); }
    template <class tag, class Arg>
    void beforeTest(Arg&& arg) { beforeTestTag(arg, tag()); }

    template <class tag, class Arg, class ... Args>
    void afterTest(Arg&& arg, Args&& ... args) { afterTest<tag>(arg); afterTest<tag>(args...); }
    template <class tag, class Arg>
    void afterTest(Arg&& arg) { afterTestTag(arg, tag()); }

    Metric& operator=(const Metric& m) { total_nb_w_treated_ptr = m.total_nb_w_treated_ptr; t_begin = m.t_begin; t_end = m.t_end; return *this; }

private:

    void beforeTestTag(time_stamp& t, parallele_exec)
    {
        total_nb_w_treated_ptr->anounceFutureOperation(); 
        t_begin = t;
    }
    void beforeTestTag(time_stamp& t, sequentiel_exec)
    {
        total_nb_w_treated_ptr->anounceFutureOperation(); 
        total_nb_w_treated_ptr->allowOperations(); 
        t_begin = t;
    }

    void afterTestTag(time_stamp& t, exec) { t_end = t; }
    void afterTestTag(unsigned nb_words_read, exec)
    {
        inner_nb_words_treated = nb_words_read;
        total_nb_w_treated_ptr->b_treatOperation(&adder<size_t>::plus, nb_words_read);
    }

};

class GlobalMetric
{

    timer t_timer;

    std::vector<Metric> execution_metrics;

    time_stamp t_created,
               t_start,
               t_end;

    prot_adder_st total_nb_word_read;

public:

    GlobalMetric() = delete;
    GlobalMetric(unsigned nb_threads) : execution_metrics{ nb_threads + 1 }, t_created{ t_timer.now() }, total_nb_word_read{ }
    {
        initThreadsMetrics();
    }

    Metric* getMetricPtr(unsigned idx) { return &execution_metrics[idx]; }

    time_stamp registerStart() { t_start = t_timer.now(); return t_start; }
    time_stamp registerEnd() { t_end = t_timer.now(); return t_end; }

    unsigned getNbThreads() const { return execution_metrics.size() - 1; }

    time_stamp getCreationTime() const { return t_created; }
    time_length getDuration() const { return t_end - t_start; }

    unsigned b_getNumberWordTreated() { total_nb_word_read.allowOperations(); return total_nb_word_read.b_getResult().t; }

    void allowNumberWordTreatedCalculation() { total_nb_word_read.allowOperations(); }

    std::vector<unsigned> getIndividualNumberWordTreated()
    {
        std::vector<unsigned> list; for_each(execution_metrics.begin(), execution_metrics.end(), [&](Metric& exec_metric)
        {
            list.push_back(exec_metric.getNbWordsTreated());
        });

        return list;
    }

    std::vector<time_length> getExecutionTimes() {
        std::vector<time_length> list; std::for_each(execution_metrics.begin(), execution_metrics.end(), [&](Metric& exec_metric)
        {
            list.push_back(exec_metric.getTestDuration());
        });

        return list;
    }

private:

    void initThreadsMetrics()
    {
        std::for_each(execution_metrics.begin(), execution_metrics.end(), [&] (Metric& metric)
        {
            metric.synchroniseAdder(total_nb_word_read);
        });
    }

};

#endif
