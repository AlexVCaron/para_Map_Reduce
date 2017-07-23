#ifndef MAP_REDUCE_H
#define MAP_REDUCE_H

#include "MapOpImpl.h"
#include "types.h"
#include "MRUtilities.h"
#include "Metrics.h"
#include "FileReader.h"

#include <map>
#include <vector>
#include <future>

using namespace std;

class map_reduce
{

    files_data total_workload;

public:

    map_reduce() = delete;
    map_reduce(files_data total_workload) : total_workload { total_workload }, f_map_impl{} { }
    map_reduce(files_data total_workload, word_inspector inspector) : total_workload{ total_workload }, f_map_impl{ ruled_map_operation_factory(inspector) } { }

    void start(map<string, unsigned>& map_out, GlobalMetric* g_metric_out, unsigned cap_to_seq = 0) {

        unsigned nb_threads = g_metric_out->getNbThreads();

        p_usubber workload_before_cap(total_workload.size - cap_to_seq);
        workload_before_cap.allowOperations();

        unsigned file_share = (nb_threads > 1) ? total_workload.size / (nb_threads - 1) : 1,
                 remaining_share = (nb_threads > 1) ? total_workload.size % (nb_threads - 1) : total_workload.size;

        p_files_data capped_files;
        capped_files.allowOperations();

        vector<map<string, unsigned>> results;
        vector<future<map<string, unsigned>>> workers(0);
        vector<map_operation_impl> work;
       
        for (unsigned i = 0; i < nb_threads - 1; ++i)
        {
            work.emplace_back(
                f_map_impl.createImpl(std::forward<files_data>(total_workload.splitLoad(i * file_share, ((i + 1) * file_share) - 1)), 
                                                                                          g_metric_out->getMetricPtr(i))
            );
        }
        
        time_stamp t_start = g_metric_out->registerStart();

        for (unsigned i = 0; i < nb_threads - 1; ++i)
        {
            workers.emplace_back(async([&](map_operation_impl* impl_ptr, p_files_data* remaining_workload_ptr) -> map<string, unsigned>
            {
                map<string, unsigned> t_map_out;

                impl_ptr->execute<parallele_exec>(t_map_out, workload_before_cap, remaining_workload_ptr);

                return std::forward<map<string, unsigned>>(t_map_out);
            }, &work[i], &capped_files));
        }

        if (remaining_share > 0) {
            results.resize(1);
            map_operation_impl seq_map_operation = f_map_impl.createImpl(total_workload.splitLoad(total_workload.size - remaining_share, total_workload.size - 1), 
                                                                                                    g_metric_out->getMetricPtr(nb_threads - 1)
                                                                          );
            seq_map_operation.execute<sequentiel_exec>(results.back());
        }
        else { g_metric_out->allowNumberWordTreatedCalculation(); }
        
        for (unsigned i = 0; i < workers.size(); ++i) {
            results.emplace_back(workers[i].get());
        }

        files_data capped_workload = capped_files.b_getResult();
        capped_workload.path = total_workload.path;

        if (capped_workload.size > 0) {
            map_operation_impl cap_map_operation = f_map_impl.createImpl(capped_workload, g_metric_out->getMetricPtr(nb_threads));
            cap_map_operation.execute<sequentiel_exec>(results.back());
        }

        if (results.size() > 0) reduce(results, reduceMapVector);

        time_stamp t_end = g_metric_out->registerEnd();

        map_out = results.front();
    }

    struct map_operation_factory
    {
        virtual map_operation_impl createImpl(files_data workload, Metric* metric_ptr)
        {
            map_operation_impl impl(workload, metric_ptr);

            return std::forward<map_operation_impl>(impl);
        }

        virtual ~map_operation_factory() = default;
    };

    struct ruled_map_operation_factory : map_operation_factory
    {
    private:
        word_inspector inspector;
    public:
        ruled_map_operation_factory() = delete;
        ruled_map_operation_factory(word_inspector inspector) : inspector{ inspector } {}

        map_operation_impl createImpl(files_data workload, Metric* metric_ptr) override
        {
            map_operation_impl impl(inspector, workload, metric_ptr);

            return std::forward<map_operation_impl>(impl);
        }
    };

private:

    map_operation_factory f_map_impl;

};

#endif