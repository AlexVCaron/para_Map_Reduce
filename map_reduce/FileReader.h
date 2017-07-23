#ifndef FILE_READER_H
#define FILE_READER_H

#include "types.h"

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <fstream>

struct word_inspector
{
private:
    rule r;
public:
    word_inspector() = delete;
    word_inspector(rule r) : r{ r } {};
    word_inspector(word_inspector& w_i) : r{ w_i.r } {}
    bool inspect(std::string s) const { return r(s); }
};

struct file_reader
{

private:

    struct file_reader_impl
    {
        file_reader_impl() {}

        virtual void checkAndEmplace(std::string word, std::map<std::string, unsigned>& s_map)
        {
            auto it = s_map.find(word);
            if (!(it == s_map.end())) it->second += 1;
            else s_map.emplace(std::make_pair(word, 1));
        }

        virtual ~file_reader_impl() = default;
    };

    struct ruled_f_r_impl : file_reader_impl
    {
    private:
        word_inspector w_i;
    public:

        ruled_f_r_impl() = delete;
        ruled_f_r_impl(word_inspector w_i) : w_i(w_i) {}
        ruled_f_r_impl(rule r) : w_i{ r } {}

        void checkAndEmplace(std::string word, std::map<std::string, unsigned>& m) override
        {
            if (w_i.inspect(word))
            {
                file_reader_impl::checkAndEmplace(word, m);
            }
        }
    };
    
    file_reader_impl reader_impl;

public:

    file_reader() : reader_impl{} {}
    file_reader(word_inspector& inspector) : reader_impl{ ruled_f_r_impl(inspector) } {}

    unsigned read(std::string file, std::map<std::string, unsigned>& map_out)
    {
        unsigned count = 0;

        std::fstream f(file);
        std::string word;

        while (f >> word)
        {
            reader_impl.checkAndEmplace(word, map_out);
            ++count;
        }

        f.close();

        return count;
    }

};

#endif