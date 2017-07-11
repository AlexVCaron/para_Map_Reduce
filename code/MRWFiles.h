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

using namespace std;



class mr_w_files
{
    timer t_timer;
    time_stamp t_created,
               t_called,
               t_start,
               t_end;
public:
    mr_w_files() { t_created = t_timer.now(); };
    mr_w_files(word_inspector& w_i, string exec_type, unsigned nb_threads = 1);
    ~mr_w_files();
    class mr_impl
    {
        template<class T>
        void execute(T&) {};
    };
private:
    mr_impl m_r;
};

class seq_mr_impl : mr_w_files::mr_impl
{
    Metric m;
    files_data f_d;
    file_reader f_r;

    seq_mr_impl(word_inspector& w_i) : f_r{ w_i } {}

    void execute(map<string, unsigned>& m_p)
    {
        unsigned nb_words_read = 0;
        vector<string> files = f_d.splitFiles(f_d.nb_files - 1, f_d.nb_files - 1);
        timer t;
        m.beforeTest(t.now());
        for_each(files.begin(), files.end(), [&](string& file) { nb_words_read += f_r.read(f_d.path + '\\' + file, m_p); });
        m.afterTest(t.now(), nb_words_read);
    }
};

class par_mr_impl : mr_w_files::mr_impl
{
    
};

#endif