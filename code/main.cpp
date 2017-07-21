
#include "MRUtilities.h"
#include "MRWFiles.h"
#include "FileReader.h"
#include <iomanip>
#include <vector>
#include "Runner.h"

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
    unsigned wanted_nb_files = atoi(argv[2]);
    if(wanted_nb_files > f_d.nb_files)
    {
        unsigned mult = log(wanted_nb_files / f_d.nb_files) / log(2) + 0.5f;
        for (i = 0; i < mult; ++i) f_d.addFiles(f_d.files);
    }

    f_dtata.close();
	cout << " | " << f_d.nb_files << " fichiers a traiter charges en memoire..." << endl;

	//S'occupe de l'exec
	// Prend le file data pour faire ses tests
    mr_w_files mr_w(f_d);
    vector<unsigned> caps{ 16, 32, 64, 128, 256, 512, 1024 };
    runner rner(mr_w, f_d.nb_files, "");
    rner(caps, ".r1.home");
    rner(caps, ".r2.home");
    rner(caps, ".r3.home");
    rner(caps, ".r4.home");
    rner(caps, ".r5.home");
    cin >> c;

}
