#ifndef MAP_OP_IMPL
#define MAP_OP_IMPL

#include "MRUtilities.h"
#include "FileReader.h"
#include "Metrics.h"
#include "MapReduce.h"

class map_operation_impl
{

    files_data workload;
    file_reader reader;

    Metric* metric;
    timer t;

    map_operation_impl(file_reader reader, files_data& workload) : workload{ workload }, reader{ reader }, metric{ nullptr }, t{} { }

public:

    map_operation_impl(files_data workload, Metric* metric) : workload{ workload }, reader{}, metric{ metric }, t{} {}
    map_operation_impl(word_inspector inspector, files_data workload, Metric* metric_ptr) : workload{ workload }, reader{ inspector }, metric{ metric_ptr }, t{} {}
    map_operation_impl(map_operation_impl& other) : map_operation_impl{ other.reader, other.workload } { metric = other.metric; t = other.t; }
    map_operation_impl(map_operation_impl&& m_other) noexcept : map_operation_impl{ m_other.reader, m_other.workload } { metric = m_other.metric; t = std::move(m_other.t); }

    template <class exec_tag, class ... Args>
    void execute(Args&& ... args)
    {
        execute(args..., exec_tag());
    }

private:

    template <class exec_tag>
    void execute(std::map<std::string, unsigned>& m_p, exec_tag e)
    {
        using tag = typename exec_traits<exec_tag>::exec_category;
        unsigned nb_words_read = 0;

        metric->beforeTest<tag>(t.now());

        for_each(workload.begin(), workload.end(), [&](string& file) {
            nb_words_read += reader.read(workload.path + '/' + file, m_p);
        });

        metric->afterTest<tag>(t.now(), nb_words_read);
    }

    template <class exec_tag>
    void execute(std::map<std::string, unsigned>& m_p, p_usubber& nb_files_treated, p_files_data* remaining_files, exec_tag e)
    {
        using tag = typename exec_traits<exec_tag>::exec_category;
        unsigned nb_words_read = 0;

        remaining_files->anounceFutureOperation();

        metric->beforeTest<tag>(t.now());

        auto last_treated_file_it = find_if(workload.begin(), workload.end(), [&](string& file) {
            if (nb_files_treated.getResult().t == 0u) return true;

            nb_files_treated.anounceFutureOperation();

            nb_words_read += reader.read(workload.path + "/" + file, m_p);

            nb_files_treated.b_treatOperation(&subber<unsigned>::minus, 1u);

            return false;
        });

        vector<string> v_i(last_treated_file_it, workload.end());
        remaining_files->b_treatOperation(&files_data::addFiles, v_i);

        metric->afterTest<tag>(t.now(), nb_words_read);
    }

};

#endif