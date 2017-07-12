#ifndef REDUCE_H
#define REDUCE_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>

struct files_data
{
    unsigned nb_files;
    std::string path;
    std::vector<std::string> files;
    files_data() : nb_files{ 0 } {}
    files_data(files_data& f_d) : nb_files{ f_d.nb_files }, path{ f_d.path }, files{ f_d.files } {}
    files_data splitFiles(int i, int j) { files_data f_d(*this); f_d.files = std::vector<std::string>(files.begin() + i, files.begin() + j + 1); return f_d; }
    std::vector<std::string>::iterator begin() { return files.begin(); }
    std::vector<std::string>::iterator end() { return files.end(); }
};

template <class T>
struct adder
{
    T t;
    adder() : t{} {}
    adder(T& t) : t{ t } {}
    adder(T&& t) : t{ t } {}
    void plus(T& o_t) { t = t + o_t; }
};

template <class T, class F>
void reduce(std::vector<T>& m_p, F& f)
{
    f(m_p);
}

inline void reduceMapVector(std::vector<std::map<std::string, unsigned>>& m_p)
{
    auto first_map = m_p.begin();

    for (auto next_map = std::next(first_map); next_map != m_p.end(); ++next_map)
    {
        std::for_each(next_map->begin(), next_map->end(), [&](auto n_pair)
        {
            auto it = first_map->find(n_pair.first);
            if (!(it == first_map->end())) it->second += n_pair.second;
            else first_map->emplace(n_pair);
        });
        next_map->clear();
    }
    m_p.resize(1);
}

template <class T>
void addVector(std::vector<T>& v_t)
{
    auto first = v_t.begin(),
        present = next(first);
    for_each(present, v_t.end(), [&](T& i) { *first += i; });
    v_t.resize(1);
}

#endif