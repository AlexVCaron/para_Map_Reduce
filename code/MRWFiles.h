#ifndef MR_W_FILES_H
#define MR_W_FILES_H

#include "types.h"
#include "MRUtilities.h"
#include "ProtectedT.h"
#include "Metrics.h"

#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include "FileReader.h"
#include <future>

using namespace std;



class mr_w_files
{
    timer t_timer;
    time_stamp t_created,
               t_start,
               t_end;
    files_data f_d;
    word_inspector w_i;

public:

    mr_w_files(word_inspector& w_i, files_data& f_d);

    template <class exec_tag, class op_impl>
    void start(map<string, unsigned>& m_p_out, unsigned nb_threads = 1);

    class map_op_impl
    {
    protected:
        files_data f_d;
        file_reader f_r;
    public:
        map_op_impl(word_inspector& w_i, files_data f_d) : f_d{ f_d }, f_r{ w_i } {}
        template<class exec_tag, class T>
        void execute(T&) {};
    };
private:
    mr_w_files() { t_created = t_timer.now(); };
};


class thread_map_op_impl : mr_w_files::map_op_impl
{
    Metric metric;
public:
    thread_map_op_impl(word_inspector& w_i, files_data f_d) : map_op_impl(w_i, f_d) {}

    template <class exec_tag>
    void execute(map<string, unsigned>& m_p)
    {
        unsigned nb_words_read = 0;
        timer t;
        metric.beforeTest<exec_tag>(t.now());
        for_each(f_d.begin(), f_d.end(), [&](string& file) { nb_words_read += f_r.read(f_d.path + '\\' + file, m_p); });
        metric.afterTest<exec_tag>(t.now(), nb_words_read);
    }
};

#endif