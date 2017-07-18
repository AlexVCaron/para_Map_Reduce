#ifndef MR_W_FILES_H
#define MR_W_FILES_H

#include "types.h"
#include "MRUtilities.h"
#include "Metrics.h"
#include "FileReader.h"

#include <map>
#include <vector>
#include <algorithm>
#include <future>

using namespace std;

class mr_w_files
{
    GlobalMetric g_m;
    files_data f_d;

public:
    mr_w_files() = delete;
    mr_w_files(files_data f_d, unsigned nb_threads = 1) : g_m{ nb_threads }, f_d { f_d }, impl{} { }
    mr_w_files(files_data f_d, word_inspector w_i, unsigned nb_threads = 1) : g_m{ nb_threads }, f_d{ f_d }, impl{ ruled_mr_w_files_impl(w_i) } { }

    GlobalMetric* getGlobalMetricPtr() { return &g_m; }

    void start(map<string, unsigned>& m_p_out) {
        unsigned nb_threads = g_m.getNbThreads();

        vector<future<map<string, unsigned>>> workers(0);
        vector<map<string, unsigned>> results;

        unsigned file_share = (nb_threads > 1) ? f_d.nb_files / (nb_threads - 1) : 1,
                 remaining_share = (nb_threads > 1) ? f_d.nb_files % (nb_threads - 1) : f_d.nb_files;

        time_stamp t_start = g_m.registerStart();
        
        vector<thread_map_op_impl> work;

        for (unsigned i = 0; i < nb_threads - 1; ++i)
        {
            work.push_back(impl.createOpImpl(std::forward<files_data>(f_d.splitFiles(i * file_share, ((i + 1) * file_share) - 1)), g_m.getMetricPtr(i)));
        }

        for (unsigned i = 0; i < nb_threads - 1; ++i)
        {
            workers.emplace_back(async([](thread_map_op_impl* pt_impl) -> map<string, unsigned>
            {
                map<string, unsigned> m_p;
                pt_impl->execute<parallele_exec>(m_p);
                return std::forward<map<string, unsigned>>(m_p);
            }, &work[i]));
        }

        if (remaining_share > 0) {
            results.resize(1);
            thread_map_op_impl mp_op = impl.createOpImpl(f_d.splitFiles(f_d.nb_files - remaining_share - 1, f_d.nb_files - 1), g_m.getMetricPtr(nb_threads - 1));
            mp_op.execute<sequentiel_exec>(results.back());
        }
        else { g_m.calculateNumberWordTreated(); }

        for (unsigned i = 0; i < workers.size(); ++i) {
            results.emplace_back(workers[i].get());
        }

        if (results.size() > 0) reduce(results, reduceMapVector);

        time_stamp t_end = g_m.registerEnd();

        m_p_out = results.front();
    }

    class thread_map_op_impl
    {
        files_data f_d;
        file_reader f_r;
        Metric metric;
        timer t;
        thread_map_op_impl(file_reader& f_r, files_data& f_d) : f_d{ f_d }, f_r{ f_r }, t{} { }
    public:
        thread_map_op_impl(files_data f_d, Metric* m) : f_d{ f_d }, f_r{ }, t{} { metric.synchroniseAdder(m); }
        thread_map_op_impl(word_inspector w_i, files_data f_d, Metric* m) : f_d{ f_d }, f_r{ w_i }, t{} { metric.synchroniseAdder(m); }
        thread_map_op_impl(thread_map_op_impl& t_m) : thread_map_op_impl{ t_m.f_r, t_m.f_d } { metric = t_m.metric; t = t_m.t; }
        thread_map_op_impl(thread_map_op_impl&& t_m) noexcept : thread_map_op_impl{ t_m.f_r, t_m.f_d } { metric = std::move(t_m.metric); t = std::move(t_m.t); }
        template <class exec_tag>
        void execute(map<string, unsigned>& m_p)
        {
            execute(m_p, exec_tag());
        }
        template <class exec_tag>
        void execute(map<string, unsigned>& m_p, exec_tag e)
        {
            using tag = typename exec_traits<exec_tag>::exec_category;
            unsigned nb_words_read = 0;
            metric.beforeTest<tag>(t.now());
            for_each(f_d.begin(), f_d.end(), [&](string& file) { nb_words_read += f_r.read(f_d.path + '\\' + file, m_p); });
            metric.afterTest<tag>(t.now(), nb_words_read);
        }
    };

    struct mr_w_files_impl
    {
        virtual ~mr_w_files_impl() = default;
        virtual thread_map_op_impl createOpImpl(files_data f_d, Metric* m)
        {
            thread_map_op_impl impl(f_d, m);
            return std::forward<thread_map_op_impl>(impl);
        }
    };
    struct ruled_mr_w_files_impl : mr_w_files_impl
    {
        word_inspector w_i;
        ruled_mr_w_files_impl() = delete;
        ruled_mr_w_files_impl(word_inspector w_i) : w_i{ w_i } {}
        thread_map_op_impl createOpImpl(files_data f_d, Metric* m)
        {
            thread_map_op_impl impl(w_i, f_d, m);
            return std::forward<thread_map_op_impl>(impl);
        }
    };

    mr_w_files_impl impl;
};

#endif