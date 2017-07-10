#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <fstream>

struct word_inspector
{
    typedef bool(*rule)(std::string);
    std::vector<rule> v_r;
    word_inspector() {}
    word_inspector(std::vector<rule> v_r) : v_r{ v_r } {}
    bool inspect(std::string& word) {
        bool b = true;
        find_if(v_r.begin(), v_r.end(), [&](rule r) { if (!(r(word))) { b = false; return true; } return false; });
        return b;
    }
};

struct file_reader
{
    word_inspector w_i;
    file_reader(word_inspector w_i) : w_i{ w_i } {}
    unsigned read(std::string file, std::map<std::string, unsigned>& m) const
    {
        unsigned count = 0;
        std::fstream f(file);
        std::string word;
        while (f >> word)
        {
            if ((const_cast<word_inspector*>(&w_i))->inspect(word))
            {
                auto it = m.find(word);
                if (!(it == m.end())) it->second += 1;
                else m.emplace(std::make_pair(word, 1));
            }
            ++count;
        }
        f.close();
        return count;
    }
private:
    file_reader() : w_i() {}
};

#endif