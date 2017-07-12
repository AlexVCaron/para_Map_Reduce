#include "MRWFiles.h"

mr_w_files::mr_w_files(word_inspector& w_i, files_data& f_d) : mr_w_files()
{
    w_i = w_i;
    f_d = f_d;
}

template <class exec_tag, class op_impl>
void mr_w_files::start(map<string, unsigned>& m_p_out, unsigned nb_threads)
{
    vector<future<map<string, unsigned>>> workers;
    vector<map<string, unsigned>> results(1);

    unsigned file_share = (nb_threads == 1) ? f_d.nb_files / (nb_threads - 1) : 1,
             remaining_share = (nb_threads == 1) ? f_d.nb_files % (nb_threads - 1) : f_d.nb_files - 1;

    t_start = t_timer.now();

    for (unsigned i = 0; i < nb_threads - 1; ++i)
    {
        workers.emplace_back(async([](map_op_impl* mp_op)
        {
            map<string, unsigned> m_p;
            mp_op->execute<exec_tag>(m_p);
            return m_p;
        }, op_impl(w_i, f_d.splitFiles(i * file_share, ((i + 1) * file_share) - 1))));
    }

    if (remaining_share > 0) {
        map_op_impl mp_op = op_impl(w_i, f_d.splitFiles(f_d.nb_files - remaining_share - 1, f_d.nb_files - 1));
        mp_op.execute<sequentiel_exec_tag>(results.back());
    }

    for (unsigned i = 0; i < workers.size(); ++i) {
        results.emplace_back(workers[i].get());
    }

    if (results.size() > 0) reduce(results, reduceMapVector);

    t_end = t_timer.now();

    m_p_out = results.front();
}
