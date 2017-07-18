#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <fstream>

struct word_inspector
{
    typedef bool(*rule)(std::string);
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
    struct f_r_impl
    {

        virtual void checkAndEmplace(std::string word, std::map<std::string, unsigned>& m)
        {
            auto it = m.find(word);
            if (!(it == m.end())) it->second += 1;
            else m.emplace(std::make_pair(word, 1));
        }
        virtual ~f_r_impl() = default;
    };
    struct ruled_f_r_impl : f_r_impl
    {
        ruled_f_r_impl() = delete;
        ruled_f_r_impl(word_inspector w_i) : w_i(w_i) {}
        ruled_f_r_impl(word_inspector::rule r) : w_i{ r } {}
        void checkAndEmplace(std::string word, std::map<std::string, unsigned>& m)
        {
            if (w_i.inspect(word))
            {
                f_r_impl::checkAndEmplace(word, m);
            }
        }
    private:
        word_inspector w_i;
    };

    f_r_impl impl;
    
    file_reader() : impl{} {}
    file_reader(word_inspector& w_i) : impl{ ruled_f_r_impl(w_i) } {}
    unsigned read(std::string file, std::map<std::string, unsigned>& m)
    {
        unsigned count = 0;
        std::fstream f(file);
        std::string word;
        while (f >> word)
        {
            impl.checkAndEmplace(word, m);
            ++count;
        }
        f.close();
        return count;
    }
};

#endif