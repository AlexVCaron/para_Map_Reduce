#ifndef MR_W_FILES_H
#define MR_W_FILES_H
/*
#include "types.h"
#include "MRUtilities.h"
#include "ProtectedT.h"

#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include "FileReader.h"

using namespace std;

using prot_adder_st = protectedT<adder<size_t>>;

class mr_w_files
{
    timer t_timer;
    time_stamp t_created,
               t_called,
               t_start,
               t_end;
public:
    mr_w_files() { t_created = t_timer.now(); };
    mr_w_files(string exec_type, unsigned nb_threads = 1) {}
    ~mr_w_files();
    class mr_impl
    {
        template<class T>
        virtual void execute(T&, time_stamp&, time_stamp&) = 0;
    };
private:
    mr_impl m_r;
};

class seq_mr_impl : mr_w_files::mr_impl
{
    prot_adder_st* nb_w_treated;
    files_data f_d;
    file_reader f_r;

    void execute(map<string, unsigned>& m_p, time_stamp& t_start, time_stamp& t_end)
    {
        unsigned nb_words_read = 0;
        vector<string> files = f_d.splitFiles(f_d.nb_files - 1, f_d.nb_files - 1);
        timer t;
        t_start = t.now();
        nb_w_treated->registerOperation();
        for_each(files.begin(), files.end(), [&](string& file) { nb_words_read += f_r.read(f_d.path + '\\' + file, m_p); });
        t_end = t.now();
        nb_w_treated->allowOperation();
        nb_w_treated->startTransformations();
        nb_w_treated->waitForTransform(&adder<size_t>::plus, nb_words_read);
    }
};

class par_mr_impl : mr_w_files::mr_impl
{
    
};
*/
#endif