/*
#include "MRUtilities.h"

#include "FileReader.h"
#include "ProtectedT.h"

#include <thread>
#include <iostream>
#include <fstream>
#include <vector>
#include<algorithm>
#include <map>
#include <future>
#include <iomanip>


using namespace std;

using timer = chrono::high_resolution_clock;
using time_length = timer::duration;
using time_unit = chrono::milliseconds;

void show(map<string, unsigned> m_p)
{
    cout << " +" << setfill('-') << setw(22) << "+" << setfill('-') << setw(14) << "+" << endl;
    cout << " | Mot" << setfill(' ') << setw(19) << " | " << "Apparition | " << endl;
    cout << " +" << setfill('-') << setw(22) << "+" << setfill('-') << setw(14) << "+" << endl;
    for_each (m_p.begin(), m_p.end(), [](auto m_pair)
    {
        cout << " | " << setfill(' ') << setw(19) << m_pair.first << " |     " << m_pair.second << setw(9 - to_string(m_pair.second).size()) << " |" << endl;
    });
    cout << " +" << setfill('-') << setw(22) << "+" << setfill('-') << setw(14) << "+" << endl;
}

void showData(map<string, unsigned>& m_p, unsigned nb_mot_traites)
{
    float divs[] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
    unsigned i = 0;
    auto p_mot_it = m_p.begin();
    cout << endl;
    cout << " | Nombre de mots dans le map : " << m_p.size() << endl;
    cout << " | Nombre de mots traites     : " << nb_mot_traites << endl;
    cout << " +" << setfill('-') << setw(47) << "+" << endl;
    cout << " |     1er mot : " << p_mot_it->first << setfill(' ') << setw(20 - p_mot_it->first.length()) << " = " << setw(5) << p_mot_it->second << " fois +" << endl;
    for_each(divs, divs + 10, [&] (float div)
    {
        while (i != int(div * m_p.size()) - 1) { ++i; ++p_mot_it; }
        cout << " | " << setfill(' ') << setw(6) << i << "e mot : " << p_mot_it->first << setfill(' ') << setw(20 - p_mot_it->first.length()) << " = " << setw(5) << p_mot_it->second << " fois +" << endl;
    });
    cout << " +" << setfill('-') << setw(47) << "+" << endl;
}

void showTime(vector<time_length>& v_t)
{
    unsigned i = 0;
    auto end = v_t.end(); --end;
    cout << endl;
    cout << " |-> Thread_main" << " : " << chrono::duration_cast<time_unit>(*end).count() << setfill(' ') << setw(8) << "  (ms)" << endl;
    if (v_t.size() > 1) cout << " +----+" << endl;
    for_each (v_t.begin(), end, [&] (time_length& t_s)
    {
        cout << "      |-> Thread_"  << i << setfill(' ') << setw(4) << " : " << chrono::duration_cast<time_unit>(t_s).count() << setw(8) << "  (ms)" << endl;
        ++i;
    });
    if(v_t.size() > 1) reduce(v_t, addVector<time_length>);
    cout << " +----+" << setfill('-') << setw(29) << "" << endl;
    cout << " | Temps de travail : " << chrono::duration_cast<time_unit>(v_t.front()).count() << " (ms)" << endl;
    cout << " +----+" << setfill('-') << setw(29) << "" << endl;
}

int main_old(int argc, char* argv[])
{
    char c;
    bool exec_parallele = string(argv[2]) == "parallele";
    unsigned nb_threads = (exec_parallele) ? 2 * thread::hardware_concurrency() : 1;

    cout << " | Execution ";

    if (exec_parallele) cout << "parallele a " << nb_threads << " threads";
    else cout << "sequentielle";

    cout << endl;

    vector<future<map<string, unsigned>>> v_th;

    ifstream f_dtata(argv[1]);
    files_data f_d;

    f_dtata >> f_d.nb_files >> f_d.path;

    f_d.files.resize(f_d.nb_files);
    unsigned i = 0;
    while (f_dtata >> f_d.files[i]) { if (i == (f_d.nb_files - 1)) break; ++i; }
    f_d.files.resize(f_d.nb_files);

    f_dtata.close();

    cout << " | " << f_d.nb_files << " fichiers a traiter" << endl;

    unsigned file_share = (exec_parallele) ? f_d.nb_files / (nb_threads - 1) : 1;

    typedef bool(*rule)(string);
    vector<rule> rules;
    word_inspector w_i(rules);

    timer g_timer;
    auto g_t_start = g_timer.now();
    
    vector<time_length> v_time(nb_threads);

    using prot_adder_st = protectedT<adder<size_t>>;
    prot_adder_st nb_w_treated(adder<size_t>(0u));

    if (exec_parallele) {
        for (i = 0; i < nb_threads - 1; ++i)
            v_th.emplace_back(async([](string path, vector<string> files, file_reader f_r, timer t, time_length* t_dur, prot_adder_st* nb_w_treated) ->  map<string, unsigned>
            {
                unsigned nb_words_read = 0;                
                map<string, unsigned> m_p;
                nb_w_treated->registerOperation();
                auto t_start = t.now();
                for_each(files.begin(), files.end(), [&](string& file) { nb_words_read += f_r.read(path + '/' + file, m_p); });
                *t_dur = t.now() - t_start;
                nb_w_treated->allowOperation();
                nb_w_treated->waitForTransform(&adder<size_t>::plus, nb_words_read);
                return std::forward<map<string, unsigned>>(m_p);
            }, f_d.path, f_d.splitFiles(i * file_share, ((i + 1) * file_share) - 1).files, file_reader(w_i), timer(), &v_time[i], &nb_w_treated));
    }
    
    file_reader f_r(w_i);

    vector<map<string, unsigned>> m_p;

    unsigned remaining_share = (exec_parallele) ? f_d.nb_files % (nb_threads - 1) : f_d.nb_files - 1;
    if (remaining_share != 0) {
        m_p.resize(1);
        unsigned nb_words_read = 0;
        vector<string> files = f_d.splitFiles(f_d.nb_files - remaining_share - 1, f_d.nb_files - 1).files;
        timer t;
        auto t_start = t.now();
        nb_w_treated.registerOperation();
        for_each(files.begin(), files.end(), [&](string& file) { nb_words_read += f_r.read(f_d.path + '/' + file, m_p[0]); });
        v_time.back() = t.now() - t_start;
        nb_w_treated.allowOperation();
        while (nb_w_treated.registeredOperations() != nb_threads && total_nb_w_treated_ptr.hasPendingTransformations()) this_thread::sleep_for(40ms);
        nb_w_treated.startTransformations();
        nb_w_treated.waitForTransform(&adder<size_t>::plus, nb_words_read);
    }
    else {
        if (exec_parallele)
        {
            while (nb_w_treated.registeredOperations() != nb_threads && total_nb_w_treated_ptr.hasPendingTransformations()) this_thread::sleep_for(40ms);
            nb_w_treated.startTransformations();
        }
        v_time.back() = time_length(0);
    }

    for (i = 0; i < v_th.size(); ++i) {
        m_p.emplace_back(v_th[i].get());
    }

    time_length g_map_time = g_timer.now() - g_t_start;

    if (m_p.size() > 1) {
        reduce(m_p, reduceMapVector);
    }

    time_length g_red_time = (g_timer.now() - g_t_start) - g_map_time;

    showData(m_p.front(), nb_w_treated.getResult().t);
    showTime(v_time);

    cout << endl;

    cout << " | Temps pour Map    : " << chrono::duration_cast<time_unit>(g_map_time).count() << endl;
    cout << " | Temps pour Reduce : " << chrono::duration_cast<time_unit>(g_red_time).count() << endl;

    cout << endl << " | Temps global      : " << chrono::duration_cast<time_unit>(g_map_time).count() + chrono::duration_cast<time_unit>(g_red_time).count() << endl;

    cin >> c;

    return 0;
}
*/