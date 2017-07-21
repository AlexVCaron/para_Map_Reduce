#pragma once
#include "MRWFiles.h"
#include <numeric>
#include <iomanip>

struct runner
{
    mr_w_files mr_w;
    unsigned nb_files;
    string f_prefix;
    runner() = delete;
    runner(mr_w_files& mr_w, unsigned nb_files, string file_name_prefix) : mr_w{ mr_w }, nb_files{ nb_files }, f_prefix{ file_name_prefix } {}

    void operator()(vector<unsigned> caps, string f_suffix = "") 
    {
        string out_seq, out_para;
        float temps_total;
        vector<unsigned> para_durations;

        //Spanner un test
        // 1) La global metric
        //		On y passe le # de thread (si juste 1 = sequentiel)
        cout << "Traitement parallele debute" << endl;

        GlobalMetric g_m_parallele_no_cap(thread::hardware_concurrency());
        map<string, unsigned> m_p_parallele;
        mr_w.start(m_p_parallele, &g_m_parallele_no_cap);

        out_para = f_prefix + "out-parallele" + f_suffix;
        cout << "\nParallele ( sans seuil )" << endl;
        createOutTestFile(out_para, m_p_parallele, g_m_parallele_no_cap.getNumberWordTreated(), g_m_parallele_no_cap.getDuration(), g_m_parallele_no_cap.getThreadsWorkTime());
        showTestData(g_m_parallele_no_cap.getNumberWordTreated(), g_m_parallele_no_cap.getDuration(), out_para, cout);

        for_each(caps.begin(), caps.end(), [&](unsigned cap)
        {
            if(cap < nb_files) para_durations.push_back(parallele_cap(cap, m_p_parallele, f_suffix));
        });

        // Sequentiel

        cout << endl << "Traitement sequentiel debute" << endl;

        GlobalMetric g_m_sequentielle(1);
        map<string, unsigned> m_p_sequentielle;
        mr_w.start(m_p_sequentielle, &g_m_sequentielle);

        out_seq = f_prefix + "out-sequentiel" + f_suffix;
        cout << endl << "Sequentielle" << endl;
        createOutTestFile(out_seq, m_p_sequentielle, g_m_sequentielle.getNumberWordTreated(), g_m_sequentielle.getDuration(), g_m_sequentielle.getThreadsWorkTime());
        showTestData(g_m_sequentielle.getNumberWordTreated(), g_m_sequentielle.getDuration(), out_seq, cout);

        temps_total = (chrono::duration_cast<time_unit>(g_m_parallele_no_cap.getDuration()).count() +
            accumulate(para_durations.begin(), para_durations.end(), unsigned(0)) +
            chrono::duration_cast<time_unit>(g_m_sequentielle.getDuration()).count());

        //Algorithme sequetiel : __% du temps total
        cout << endl << "Algorithme sequentiel :";
        cout << chrono::duration_cast<time_unit>(g_m_sequentielle.getDuration()).count() / temps_total * 100.f << "% du temps total" << endl;

        //Algorithme parallèles :
        cout << endl << "Algorithmes paralleles :" << endl;

        // sans seuil sequentiel
        cout << "	sans seuil sequentiel, " << chrono::duration_cast<time_unit>(g_m_parallele_no_cap.getDuration()).count() / temps_total * 100.f << "% du temps total" << endl;

        unsigned i = 0;
        for_each(para_durations.begin(), para_durations.end(), [&](unsigned& dur) {
            cout << "	seuil sequentiel " << caps[i] << "  , " << dur / temps_total * 100.f << "% du temps total" << endl;
            ++i;
        });
    }
private:
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

    void showTestData(unsigned nb_mot_traites, time_length duration, string test_out_filename, ostream &out_stream)
    {
        out_stream << nb_files << " fichiers, " << nb_mot_traites << " mots, " << chrono::duration_cast<time_unit>(duration).count() << " ms." << endl;
        if (&out_stream == &cout) out_stream << "Resultats detailles dans \"" << test_out_filename << "\"" << endl;
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
            showTestData(nb_mot_traite, duration, filename, myfile);
            showData(m_p, nb_mot_traite, myfile);
            showTime(v_t, myfile);
        }
        else cout << "Unable to open file";
    }

    unsigned int parallele_cap(unsigned cap, map<string, unsigned> &m_p_parallele, string f_suffix = "")
    {
        GlobalMetric g_m_parallele_cap(thread::hardware_concurrency());
        m_p_parallele.clear();
        mr_w.start(m_p_parallele, &g_m_parallele_cap, cap);

        string out_para = f_prefix + "out-parallele-" + to_string(cap) + f_suffix;
        cout << "\nParallele ( seuil " + to_string(cap) + " )" << endl;
        createOutTestFile(out_para, m_p_parallele, g_m_parallele_cap.getNumberWordTreated(), g_m_parallele_cap.getDuration(), g_m_parallele_cap.getThreadsWorkTime());
        showTestData(g_m_parallele_cap.getNumberWordTreated(), g_m_parallele_cap.getDuration(), out_para, cout);
        return chrono::duration_cast<time_unit>(g_m_parallele_cap.getDuration()).count();
    }
};
