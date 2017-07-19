
#include "MRUtilities.h"
#include "MRWFiles.h"
#include "FileReader.h"
#include <iomanip>

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
    for_each(divs, divs + 10, [&](float div)
    {
        while (i != int(div * m_p.size()) - 1) { ++i; ++p_mot_it; }
        cout << " | " << setfill(' ') << setw(6) << i << "e mot : " << p_mot_it->first << setfill(' ') << setw(20 - p_mot_it->first.length()) << " = " << setw(5) << p_mot_it->second << " fois +" << endl;
    });
    cout << " +" << setfill('-') << setw(47) << "+" << endl;
}

int main(int argc, char* argv[])
{
    char c;
    ifstream f_dtata(argv[1]);
    files_data f_d;

    f_dtata >> f_d.nb_files >> f_d.path;

    f_d.files.resize(f_d.nb_files);
    unsigned i = 0;
    while (f_dtata >> f_d.files[i]) { if (i == (f_d.nb_files - 1)) break; ++i; }
    f_d.files.resize(f_d.nb_files);

    f_dtata.close();

    mr_w_files mr_w(f_d);

    GlobalMetric g_m_parallele(thread::hardware_concurrency());
    map<string, unsigned> m_p_parallele;
    mr_w.start(m_p_parallele, &g_m_parallele);

    GlobalMetric g_m_sequentielle(1);
    map<string, unsigned> m_p_sequentielle;
    mr_w.start(m_p_sequentielle, &g_m_sequentielle);

    cout << "Sequentielle" << endl;
    showData(m_p_sequentielle, g_m_sequentielle.getNumberWordTreated());

    cout << "Parallele" << endl;
    showData(m_p_parallele, g_m_parallele.getNumberWordTreated());

    cin >> c;

}
