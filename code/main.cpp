
#include "MRUtilities.h"
#include "MapReduce.h"
#include "Runner.h"
#include <iomanip>
#include <vector>
#include <sstream>
#include <string>

struct in_args
{
    string config_file;
    string cout_log = "cout.log";
    string out_path_data = "";
    string data_files_suffix = "";
    string mode = "static";
    unsigned nb_threads_max = thread::hardware_concurrency();
    unsigned wanted_nb_files = 0;
    unsigned cap_max = 1024;
    unsigned nb_iter_runner = 1;

};

bool checkIfVarSuffix(string suffix, string& pre, string& post, string& mid)
{
    stringstream ss{ suffix };
    stringstream::char_type c, tmp;
    while(ss >> c)
    {
        if (c == stringstream::char_type('{'))
        {
            tmp = c;
            string tag = "";
            bool contain_tag = false;
            while(ss >> c && c != stringstream::char_type('}'))
            {
                tag += c;
                if (tag == "i_s#") {
                    contain_tag = true;
                    tag = "";
                }
            }
            if (contain_tag)
            {
                mid = tag;
                while (ss >> c)
                {
                    post += c;
                }
                return true;
            }
            pre += tmp + tag + c;
        }
        else pre += c;
    }

    return false;
}

int main(int argc, char* argv[])
{
    in_args parseInArgs(int, char*[]);
    void exec(in_args&, files_data&);

    files_data f_d;

    in_args args = parseInArgs(argc, argv);

    ifstream f_dtata(args.config_file);

    f_dtata >> f_d.size >> f_d.path;

    f_d.files.resize(f_d.size);

    unsigned i = 0;
    while (f_dtata >> f_d.files[i]) { if (i == (f_d.size - 1)) break; ++i; }

    f_dtata.close();

    f_d.files.resize(f_d.size);

    if (args.wanted_nb_files == 0u) args.wanted_nb_files = f_d.size;

    if (args.mode == "static") {
        if (args.wanted_nb_files > f_d.size)
        {
            unsigned mult = log(float(args.wanted_nb_files) / float(f_d.size)) / log(2) + 0.5f;
            for (i = 0; i <= mult; ++i) f_d.addFiles(f_d.files);
        }
        exec(args, f_d);
    }
    else if (args.mode == "exp")
    {
        unsigned mult = log(float(args.wanted_nb_files) / float(f_d.size)) / log(2) + 0.5f;
        exec(args, f_d);
        for (i = 0; i <= mult; ++i) {
            f_d.addFiles(f_d.files);
            exec(args, f_d);
        }
    }
    else if (args.mode == "lin")
    {
        while (f_d.files.size() < args.wanted_nb_files) {
            f_d.addFiles(f_d.files);
            exec(args, f_d);
        }
        f_d.addFiles(f_d.files);
        exec(args, f_d);
    }
}


void exec(in_args& args, files_data& f_d)
{
    cout << " | " << f_d.size << " fichiers a traiter charges en memoire..." << endl;

    //S'occupe de l'exec
    // Prend le file data pour faire ses tests
    map_reduce mr_w(f_d);

    vector<unsigned> caps;
    unsigned i = 0; unsigned i_pow = pow(2, i);
    while (i_pow < args.cap_max)
    {
        caps.push_back(i_pow);
        ++i;
        i_pow = pow(2, i);
    }

    vector<unsigned> nb_threads;
    for (i = 2; i <= min(args.nb_threads_max, thread::hardware_concurrency()); ++i) { nb_threads.push_back(i); }

    string pre_suffix = "",
           mid_suffix = "",
           post_suffix = "";

    bool var_suffix = checkIfVarSuffix(args.data_files_suffix, pre_suffix, post_suffix, mid_suffix);

    runner rner(mr_w, f_d.size, args.cout_log);

    for (i = 0; i < args.nb_iter_runner; ++i) {
        rner(caps, nb_threads, args.out_path_data, pre_suffix + (var_suffix ? "" : ".") + to_string(i) + mid_suffix + post_suffix);
    }
}

struct BadArgCountException {};
struct BadArgExeception {};

string extractArg(stringstream& ss)
{
    string s = "";
    char c;
    while (ss.get(c) && c != ' ' && c != '-')
    {
        s += c;
    }
    if (c == '-') ss.unget();
    if (s.front() == '\"') s = s.substr(1, s.size() - 1);
    if (s.back() == '\"') s.pop_back();
    return s;
}

in_args parseInArgs(int argc, char* argv[])
{
    if (argc < 2) throw BadArgCountException();
    stringstream ss;
    in_args args;
    args.config_file = argv[1];
    for (int i = 2; i < argc; ++i) ss << argv[i];
    if (!ss.str().empty())
    {
        bool param_flag = false;
        stringstream::char_type c;
        while(ss.get(c).good())
        {
            if (c == '-') param_flag = true;
            else if (param_flag == true)
            {
                stringstream::char_type u;
                switch(c)
                {
                case stringstream::char_type('i'):
                    ss.get(u); if (u != ' ') ss.unget();
                    args.nb_iter_runner = stoul(extractArg(ss).c_str());
                    break;
                case stringstream::char_type('o'):
                    ss.get(u); if (u != ' ') ss.unget();
                    args.out_path_data = extractArg(ss);
                    break;
                case stringstream::char_type('t'):
                    ss.get(u); if (u != ' ') ss.unget();
                    args.nb_threads_max = stoul(extractArg(ss).c_str());
                    break;
                case stringstream::char_type('n'):
                    ss.get(u); if (u != ' ') ss.unget();
                    args.wanted_nb_files = stoul(extractArg(ss).c_str());
                    break;
                case stringstream::char_type('m'):
                    ss.get(u); if (u != ' ') ss.unget();
                    args.mode = extractArg(ss).c_str();
                    break;
                case stringstream::char_type('c'):
                    ss.get(u); if (u != ' ') ss.unget();
                    args.cap_max = stoul(extractArg(ss).c_str());
                    break;
                case stringstream::char_type('s'):
                    ss.get(u); if (u != ' ') ss.unget();
                    args.data_files_suffix = extractArg(ss);
                    break;
                case stringstream::char_type('l'):
                    ss.get(u); if (u != ' ') ss.unget();
                    args.cout_log = extractArg(ss);
                    break;
                default:
                    throw BadArgExeception();
                }
                param_flag = false;
            }
        }
    }
    return args;
}