#pragma once
#include "MRWFiles.h"
#include "tLogger.h"
#include <numeric>
#include <iomanip>

struct runner
{
    mr_w_files mr_w;
    unsigned nb_files;
    t_logger<decltype(cout)> cout_log;
    runner() = delete;
    runner(mr_w_files& mr_w, unsigned nb_files, string cout_log_f_name) : 
        mr_w{ mr_w }, 
        nb_files{ nb_files }, 
        cout_log{ make_tLogger(cout, cout_log_f_name) }
    { }

    void operator()(vector<unsigned>& caps, vector<unsigned>& nb_threads, string data_path = "", string f_suffix = "", string rule_message = " ") 
    {
        string out_seq;
        float temps_total;

        vector<pair<string, vector<pair<unsigned, unsigned>>>> exec_times;

        vector<unsigned> para_durations;

        map<string, unsigned> m_p_exec;

        GlobalMetric g_m_scrap_run(thread::hardware_concurrency());
        unsigned nb_words_read_ref;

        mr_w.start(m_p_exec, &g_m_scrap_run);
        nb_words_read_ref = g_m_scrap_run.getNumberWordTreated();
        m_p_exec.clear();

        //Spanner un test
        // 1) La global metric
        //		On y passe le # de thread (si juste 1 = sequentiel)
        cout_log << "\nTraitement parallele debute\n";

        vector<pair<unsigned, unsigned>> times;

        for_each(nb_threads.begin(), nb_threads.end(), [&](unsigned nb_ths)
        {
            times.clear();
            GlobalMetric g_m_parallele_no_cap(nb_ths);

            mr_w.start(m_p_exec, &g_m_parallele_no_cap);
            if (nb_words_read_ref != g_m_parallele_no_cap.getNumberWordTreated()) throw BadWordReadExecption();
            string out_para = data_path + (data_path == "" ? "" : "/") + "exec_" + to_string(nb_files) + "_out_parallele_" + to_string(nb_ths) + "th_uncap" + f_suffix;
            cout_log << "\nParallele_" << nb_ths << "_threads ( sans seuil )\n";
            createOutTestFile(out_para, m_p_exec, g_m_parallele_no_cap.getNumberWordTreated(), g_m_parallele_no_cap.getDuration(), g_m_parallele_no_cap.getThreadsWorkTime());
            showTestData(g_m_parallele_no_cap.getNumberWordTreated(), g_m_parallele_no_cap.getDuration(), out_para);
            para_durations.push_back(chrono::duration_cast<time_unit>(g_m_parallele_no_cap.getDuration()).count());

            times.emplace_back(0u, unsigned(chrono::duration_cast<time_unit>(g_m_parallele_no_cap.getDuration()).count()));

            run(nb_ths, m_p_exec, nb_words_read_ref, para_durations, times, caps, data_path, f_suffix);

            exec_times.emplace_back(make_pair("parallele_" + to_string(nb_ths), times));
        });
        times.clear();
        // Sequentiel

        cout_log << "\nTraitement sequentiel debute\n";

        GlobalMetric g_m_sequentielle(1);
        mr_w.start(m_p_exec, &g_m_sequentielle);

        out_seq = data_path + (data_path == "" ? "" : "/") + "exec_" + to_string(nb_files) + "_out_sequentiel" + f_suffix;
        cout_log << "\nSequentielle\n";
        createOutTestFile(out_seq, m_p_exec, g_m_sequentielle.getNumberWordTreated(), g_m_sequentielle.getDuration(), g_m_sequentielle.getThreadsWorkTime());
        showTestData(g_m_sequentielle.getNumberWordTreated(), g_m_sequentielle.getDuration(), out_seq);

        times.emplace_back(0u, unsigned(chrono::duration_cast<time_unit>(g_m_sequentielle.getDuration()).count()));
        exec_times.emplace_back(make_pair("sequentielle", times));

        times.clear();

        temps_total = (accumulate(para_durations.begin(), para_durations.end(), unsigned(0)) +
                       chrono::duration_cast<time_unit>(g_m_sequentielle.getDuration()).count());

        //Algorithme sequetiel : __% du temps total
        cout_log << "\nAlgorithme sequentiel :";
        cout_log << chrono::duration_cast<time_unit>(g_m_sequentielle.getDuration()).count() / temps_total * 100.f << "% du temps total\n";

        for (int j = 0; j < nb_threads.size(); ++j) {
            unsigned i = 0;
            //Algorithme parallèles :
            cout_log << "\nAlgorithmes paralleles " << nb_threads[j] << " threads :\n";

            // sans seuil sequentiel
            cout_log << "	sans seuil sequentiel, " << para_durations[i] / temps_total * 100.f << "% du temps total\n";

            for_each(caps.begin(), caps.end(), [&](unsigned& cap) {
                if (cap <= nb_files) {
                    cout_log << "	seuil sequentiel " << cap << "  , " << para_durations[i + j + 1] / temps_total * 100.f << "% du temps total\n";
                    ++i;
                }
            });
        }

        writeDataCSV(exec_times, nb_words_read_ref, data_path, f_suffix, rule_message);
    }
private:

    void validate_exec() {}

    void writeDataCSV(vector<pair<string, vector<pair<unsigned, unsigned>>>>& exec_times,
                      unsigned nb_total_mots, string data_path = "", string f_suffix = "", string rule_comment = " ", string time_unit = "ms")
    {
        string ruled = (rule_comment == " ") ? "unruled" : "ruled";  ofstream o_csv;
        string f_name = data_path + (data_path == "" ? "" : "/") + "exec_" + to_string(nb_files) + ruled + f_suffix + ".csv";
        if(file_exist(f_name)) o_csv.open(f_name, ios::app);
        else o_csv.open(f_name);

        o_csv << "nb_total_mots," << nb_total_mots << endl;

        for (unsigned i = 0; i < exec_times.size(); ++i)
        {
            string s1 = "", s2 = "";
            o_csv << "tag_exec," << exec_times[i].first << endl;
            if (rule_comment != " ") o_csv << "regle," << rule_comment << endl;
            s1 += "temps_exec," + time_unit + ",";
            s2 += "caps,";
            for (unsigned j = 0; j < exec_times[i].second.size(); ++j) {
                    s1 += to_string(exec_times[i].second[j].second) + ","; s2 += to_string(exec_times[i].second[j].first) + ",";
            }
            s1.pop_back(); s2.pop_back();
            o_csv << s1 << endl << s2;
            if (i != exec_times.size()) o_csv << endl;
        }
        o_csv.close();
    }

    class BadWordReadExecption {};

    void run(unsigned nb_threads, map<string, unsigned>& m_p_exec, unsigned nb_words_read_ref, vector<unsigned>& para_durations, vector<pair<unsigned, unsigned>>& times, vector<unsigned> caps, string data_path, string f_suffix)
    {
        for_each(caps.begin(), caps.end(), [&](unsigned cap)
        {
            if (cap < nb_files) {
                para_durations.push_back(parallele_cap(nb_threads, cap, m_p_exec, nb_words_read_ref, data_path, f_suffix));
                times.emplace_back(cap, para_durations.back());
            }
        });
    }

    void show(map<string, unsigned> m_p)
    {
        cout << " +" << setfill('-') << setw(22) << "+" << setfill('-') << setw(14) << "+" << endl;
        cout << " | Mot" << setfill(' ') << setw(19) << " | " << "Apparition | " << endl;
        cout << " +" << setfill('-') << setw(22) << "+" << setfill('-') << setw(14) << "+" << endl;
        for_each(m_p.begin(), m_p.end(), [](auto m_pair)
        {
            cout << " | " << setfill(' ') << setw(19) << m_pair.first << " |     " << m_pair.second << setw(9 - to_string(m_pair.second).size()) << " |" << endl;
        });
        cout << " +" << setfill('-') << setw(22) << "+" << setfill('-') << setw(14) << "+" << endl;
    }

    void showData(map<string, unsigned>& m_p, unsigned nb_mot_traites, ostream &out_stream)
    {
        float divs[] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
        unsigned i = 0;
        auto p_mot_it = m_p.begin();
        out_stream << endl;
        out_stream << " | Nombre de mots dans le map : " << m_p.size() << endl;
        out_stream << " | Nombre de mots traites     : " << nb_mot_traites << endl;
        out_stream << " +" << setfill('-') << setw(47) << "+" << endl;
        out_stream << " |     1er mot : " << p_mot_it->first << setfill(' ') << setw(20 - p_mot_it->first.length()) << " = " << setw(5) << p_mot_it->second << " fois +" << endl;
        for_each(divs, divs + 10, [&](float div)
        {
            while (i != int(div * m_p.size()) - 1) { ++i; ++p_mot_it; }
            out_stream << " | " << setfill(' ') << setw(6) << i << "e mot : " << p_mot_it->first << setfill(' ') << setw(20 - p_mot_it->first.length()) << " = " << setw(5) << p_mot_it->second << " fois +" << endl;
        });
        out_stream << " +" << setfill('-') << setw(47) << "+" << endl;
    }

    void showTestData(unsigned nb_mot_traites, time_length duration, ostream &out_stream)
    {
        out_stream << nb_files << " fichiers, " << nb_mot_traites << " mots, " << chrono::duration_cast<time_unit>(duration).count() << " ms." << endl;
    }

    void showTestData(unsigned nb_mot_traites, time_length duration, string test_out_filename)
    {
        cout_log << nb_files << " fichiers, " << nb_mot_traites << " mots, " << chrono::duration_cast<time_unit>(duration).count() << " ms.\n";
        cout_log << "Resultats detailles dans \"" << test_out_filename << "\"\n";
    }

    void showTime(vector<time_length>& v_t, ostream &out_stream)
    {
        unsigned i = 0;
        auto end = v_t.end(); --end;
        auto main_ts = *end; --end;
        main_ts += *end;
        out_stream << endl;
        out_stream << " +----+" << setfill('-') << setw(29) << "" << endl;
        out_stream << " |-> Thread_main" << " : " << chrono::duration_cast<time_unit>(main_ts).count() << setfill(' ') << setw(8) << "  (ms)" << endl;
        if (v_t.size() > 2) out_stream << " +----+" << setfill('-') << setw(29) << "" << endl;
        for_each(v_t.begin(), end, [&](time_length& t_s)
        {
            out_stream << "      |-> Thread_" << i << setfill(' ') << setw(4) << " : " << chrono::duration_cast<time_unit>(t_s).count() << setw(8) << "  (ms)" << endl;
            ++i;
        });
        if (v_t.size() > 1) reduce(v_t, addVector<time_length>);
        out_stream << " +----+" << setfill('-') << setw(29) << "" << endl;
        out_stream << " | Temps de travail : " << chrono::duration_cast<time_unit>(v_t.front()).count() << " (ms)" << endl;
        out_stream << " +----+" << setfill('-') << setw(29) << "" << endl;
    }

    void createOutTestFile(string filename, map<string, unsigned>& m_p, unsigned nb_mot_traite, time_length duration, vector<time_length> v_t)
    {
        ofstream myfile(filename);
        if (myfile.is_open())
        {
            showTestData(nb_mot_traite, duration, myfile);
            showData(m_p, nb_mot_traite, myfile);
            showTime(v_t, myfile);
        }
        else cout << "Unable to open file";
    }

    unsigned int parallele_cap(unsigned nb_threads, unsigned cap, map<string, unsigned> &m_p_parallele, unsigned supposed_nb_word_read, string data_path = "", string f_suffix = "")
    {
        GlobalMetric g_m_parallele_cap(nb_threads);
        m_p_parallele.clear();
        mr_w.start(m_p_parallele, &g_m_parallele_cap, cap);
        if (supposed_nb_word_read != g_m_parallele_cap.getNumberWordTreated()) throw BadWordReadExecption();
        string out_para = data_path + (data_path == "" ? "" : "/") + "exec_" + to_string(nb_files) + "_out_parallele_" + to_string(nb_threads) + "th_" + to_string(cap) + f_suffix;
        cout_log << "\nParallele_" << nb_threads << "_threads ( seuil " + to_string(cap) + " )\n";
        createOutTestFile(out_para, m_p_parallele, g_m_parallele_cap.getNumberWordTreated(), g_m_parallele_cap.getDuration(), g_m_parallele_cap.getThreadsWorkTime());
        showTestData(g_m_parallele_cap.getNumberWordTreated(), g_m_parallele_cap.getDuration(), out_para);
        return chrono::duration_cast<time_unit>(g_m_parallele_cap.getDuration()).count();
    }
};
